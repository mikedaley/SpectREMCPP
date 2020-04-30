Attribute VB_Name = "modTZX"
' /*******************************************************************************
'   modTAP.bas within vbSpec.vbp
'
'   Routines to handle loading of ".TZX" files (Spectrum tape images)
'
'   Authors: Mark Woodmass <mark.woodmass@ntlworld.com>
'            Paul Dunn <paul.dunn4@ntlworld.com>
'
'   This program is free software; you can redistribute it and/or
'   modify it under the terms of the GNU General Public License
'   as published by the Free Software Foundation; either version 2
'   of the License, or (at your option) any later version.
'   This program is distributed in the hope that it will be useful,
'   but WITHOUT ANY WARRANTY; without even the implied warranty of
'   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'   GNU General Public License for more details.
'
'   You should have received a copy of the GNU General Public License
'   along with this program; if not, write to the Free Software
'   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
'
' *******************************************************************************/
Option Explicit

Public gbTZXInserted As Long, gbTZXPlaying As Long
Public glEarBit As Long
Public TZXCurBlock As Long
Public TZXNumBlocks As Long

Private TZXArray() As Long
Private TZXOffsets() As Long
Private TZXBlockLength() As Long
Private BitValues(7) As Long

Private TZXBlockIsStandardTiming As Boolean
Private TZXCallList() As Long
Private TZXCallCounter As Long, TZXNumCalls As Long, TZXCallByte As Long
Private TZXTotalTs As Long

Private TZXState As Long, TZXAimTStates As Long
Private TZXPointer As Long, TZXCurBlockID As Long
Private TZXPulseLen As Long, TZXSync1Len As Long, TZXSync2Len As Long
Private TZXZeroLen As Long, TZXOneLen As Long, TZXPauseLen As Long
Private TZXDataLen As Long, TZXROMDataLen As Long, TZXUsedBits As Long
Private TZXByte As Long
Private TZXDataPos As Long, TZXPulsesDone As Long
Private TZXPulseToneLen As Long, TZXBitLimit As Long, TZXBitCounter As Long
Private TZXLoopCounter As Long, TZXLoopPoint As Long

' ////////////////////////////////////////////////////////////////////////////////
' // GetTZXBlockInfo()
' //
' // Retreives information about a specific TZX block in the current file
' //
' // lBlockNum  IN      Number of block to retrieve information on (zero based)
' // lType      OUT     The type of block (see the TZX specification document)
' // sText      OUT     Human-readable text describing the block
' // lLen       OUT     Length of the block in bytes
Public Sub GetTZXBlockInfo(lBlockNum As Long, lType As Long, sText As String, lLen As Long)
    Dim lPtr As Long, l As Long
    
    lPtr = TZXOffsets(lBlockNum)
    lType = TZXArray(lPtr)
    
    Select Case lType
    Case &H10
        sText = "Standard Block"
    Case &H11
        sText = "Turbo Block"
    Case &H12
        sText = "Pure Tone"
    Case &H13
        sText = "Pulse Sequence"
    Case &H14
        sText = "Pure Data Block"
    Case &H15
        sText = "Direct Recording"
    Case &H16
        sText = "C64 Standard Block"
    Case &H17
        sText = "C64 Turbo Block"
    Case &H20
        ' // Pause/StopTape
        l = TZXArray(lPtr + 1) + (TZXArray(lPtr + 2) * 256&)
        If l = 0 Then
            sText = "Stop Tape"
        Else
            sText = "Pause Tape for " & CStr(l) & "ms"
        End If
    Case &H21
        sText = "Group Start"
    Case &H22
        sText = "Group End"
    Case &H23
        sText = "Jump to Block"
    Case &H24
        sText = "Loop Start"
    Case &H25
        sText = "Loop End"
    Case &H2A
        sText = "Stop Tape if 48K"
    Case &H30
        sText = ""
        l = TZXArray(lPtr + 1)
        For l = lPtr + 2 To lPtr + 1 + l
            sText = sText & Chr(TZXArray(l))
        Next l
    Case &H31
        sText = "Message Block"
    Case &H32
        sText = "Archive Info"
    Case &H33
        sText = "Hardware Type"
    Case &H34
        sText = "Emulation Info"
    Case &H35
        sText = "Custom Info Block"
    Case &H40
        sText = "Snapshot Block"
    Case &H5A
        sText = "Block Merge Marker"
    Case &HFE
        sText = "End of Tape"
    End Select
    
    lLen = TZXBlockLength(lBlockNum)
End Sub

Public Sub StartTape()
  gbTZXPlaying = True
  TZXTotalTs = 0
  glEarBit = 0
End Sub

Public Sub StopTape()
    If gbTZXPlaying Then gbTZXPlaying = False
End Sub

Public Sub StartStopTape()
    If gbTZXPlaying Then StopTape Else StartTape
End Sub

Public Sub OpenTZXFile(sName As String)
    Dim ReadLength As Long
    Dim s As String, b As Long, lCounter As Long
    Dim F As Long, BlockLen As Long, BlockID As Long, ArrayLength As Long
    Dim BlockList(2048) As Long
    Dim BlockListNum As Long
    Dim BlockLengths(2048) As Long
    Dim BlockLengthsNum As Long
    
    Dim hTZXFile As Long

    Let b = 1
    For F = 0 To 7
        BitValues(F) = b
        Let b = b * 2
    Next F
    
    ' // If we currently have a TAP file open, then close it
    CloseTAPFile
    
    If Dir$(sName) = "" Then Exit Sub
    
    hTZXFile = FreeFile
    Open sName For Binary As hTZXFile
    
    ReadLength = LOF(hTZXFile)
    If ReadLength = 0 Then
        Close #hTZXFile
        Exit Sub
    End If
    
    frmMainWnd.NewCaption = App.ProductName & " - " & GetFilePart(sName)

    ' Read the TZX file into TZXArray
    ReDim TZXArray(ReadLength + 1)
    
    On Error Resume Next
    
    s = Input(ReadLength, #hTZXFile)
    For lCounter = 1 To Len(s)
        TZXArray(lCounter - 1) = Asc(Mid$(s, lCounter, 1))
    Next lCounter
    TZXArray(ReadLength) = &HFE&    ' end-of-tape block
    
    Close #hTZXFile

    ' Now decode the TZX file into an individual blocks list
    gbTZXPlaying = False
    gbTZXInserted = False
  
    s = ""
    ArrayLength = ReadLength + 1
  
    For F = 0 To 6
        s = s & Chr(TZXArray(F))
    Next F
  
    If s <> "ZXTape!" Then
        Close #hTZXFile
    End If
  
    BlockListNum = 0
    BlockLengthsNum = 0
    gbTZXInserted = True
    F = 10
    
    Do
        BlockID = TZXArray(F)
        BlockList(BlockListNum) = F
        BlockListNum = BlockListNum + 1

        F = F + 1

        Select Case BlockID
            Case &H10: BlockLen = 256& * TZXArray(F + 3) + TZXArray(F + 2) + 4
            Case &H11: BlockLen = TZXArray(F + 15) + (TZXArray(F + 16) * 256&) + (TZXArray(F + 17) * 65536) + 18
            Case &H12: BlockLen = 4
            Case &H13: BlockLen = 1 + (TZXArray(F) * 2)
            Case &H14: BlockLen = TZXArray(F + 7) + (TZXArray(F + 8) * 256&) + (TZXArray(F + 9) * 65536) + 10
            Case &H15: BlockLen = TZXArray(F + 5) + (TZXArray(F + 6) * 256&) + (TZXArray(F + 7) * 65536) + 8
            Case &H20: BlockLen = 2
            Case &H21: BlockLen = TZXArray(F) + 1
            Case &H22: BlockLen = 0
            Case &H23: BlockLen = 2
            Case &H24: BlockLen = 2
            Case &H25: BlockLen = 0
            Case &H26: BlockLen = (TZXArray(F) + (TZXArray(F + 1) * 256&) * 2) + 2
            Case &H27: BlockLen = 0
            Case &H28: BlockLen = TZXArray(F) + (TZXArray(F + 1) * 256&) + 2
            Case &H2A: BlockLen = 4
            Case &H30: BlockLen = TZXArray(F) + 1
            Case &H31: BlockLen = TZXArray(F + 1) + 2
            Case &H32: BlockLen = TZXArray(F) + (TZXArray(F + 1) * 256&) + 2
            Case &H33: BlockLen = (TZXArray(F) * 3) + 1
            Case &H34: BlockLen = 8
            Case &H35: BlockLen = TZXArray(F + 16) + (TZXArray(F + 17) * 256&) + (TZXArray(F + 18) * 65536) + (TZXArray(F + 19) * 16777216) + 20
            Case &H40: BlockLen = TZXArray(F + 1) + (TZXArray(F + 2) * 256&) + (TZXArray(F + 3) * 65536) + 4
            Case &H5A: BlockLen = 9
            Case &HFE: BlockLen = 0
            Case &HFF: BlockLen = 0
            Case Else: BlockLen = TZXArray(F) + (TZXArray(F + 1) * 256&) + (TZXArray(F + 2) * 65536) + (TZXArray(F + 3) * 16777216) + 4
        End Select
     
        F = F + BlockLen
        BlockLengths(BlockLengthsNum) = BlockLen + 1
        BlockLengthsNum = BlockLengthsNum + 1
    Loop Until F >= ArrayLength
    
    TZXNumBlocks = BlockListNum
     
    ReDim TZXOffsets(TZXNumBlocks)
    ReDim TZXBlockLength(TZXNumBlocks)

    For F = 0 To TZXNumBlocks - 1
        TZXOffsets(F) = BlockList(F)
        TZXBlockLength(F) = BlockLengths(F)
    Next F
    SetCurTZXBlock 0
    
    frmTapePlayer.UpdateTapeList
End Sub

Public Sub SetCurTZXBlock(BlockNum As Long)
    Dim F As Long
    
    TZXBlockIsStandardTiming = False
    TZXState = 5
    TZXAimTStates = 0
    TZXPointer = TZXOffsets(BlockNum)
    TZXCurBlockID = TZXArray(TZXPointer)
    Select Case TZXCurBlockID
        Case &H10: ' Standard ROM Loader block
            TZXPulseLen = 2168
            If TZXArray(TZXPointer + 5) = &HFF Then TZXPulseToneLen = 3220 Else TZXPulseToneLen = 8064
            TZXSync1Len = 667
            TZXSync2Len = 735
            TZXZeroLen = 855
            TZXOneLen = 1710
            TZXPauseLen = TZXArray(TZXPointer + 1) + (TZXArray(TZXPointer + 2) * 256&)
            TZXDataLen = TZXBlockLength(BlockNum) + TZXOffsets(BlockNum)
            TZXROMDataLen = TZXArray(TZXPointer + 3) + (TZXArray(TZXPointer + 4) * 256&)
            TZXUsedBits = 8
            TZXState = 0 ' State 0 - playing Pulse
            TZXAimTStates = TZXPulseLen
            TZXByte = 0
            TZXCurBlock = BlockNum
            TZXDataPos = TZXPointer + 5
            TZXPulsesDone = 2
            TZXBlockIsStandardTiming = True
        
        Case &H11:  ' Non-Standard TAP block
            TZXPulseLen = TZXArray(TZXPointer + 1) + (TZXArray(TZXPointer + 2) * 256&)
            TZXPulseToneLen = TZXArray(TZXPointer + 11) + (TZXArray(TZXPointer + 12) * 256&)
            TZXSync1Len = TZXArray(TZXPointer + 3) + (TZXArray(TZXPointer + 4) * 256&)
            TZXSync2Len = TZXArray(TZXPointer + 5) + (TZXArray(TZXPointer + 6) * 256&)
            TZXZeroLen = TZXArray(TZXPointer + 7) + (TZXArray(TZXPointer + 8) * 256&)
            TZXOneLen = TZXArray(TZXPointer + 9) + (TZXArray(TZXPointer + 10) * 256&)
            TZXUsedBits = TZXArray(TZXPointer + 13)
            TZXPauseLen = TZXArray(TZXPointer + 14) + (TZXArray(TZXPointer + 15) * 256&)
            TZXState = 0 ' State 0 - playing Pulse.
            TZXAimTStates = TZXPulseLen
            TZXByte = 0
            TZXCurBlock = BlockNum
            TZXDataPos = TZXPointer + 19
            TZXDataLen = TZXBlockLength(BlockNum) + TZXOffsets(BlockNum)
            TZXROMDataLen = TZXArray(TZXPointer + 3) + (TZXArray(TZXPointer + 4) * 256&)
            If (TZXPulseLen = 2168) And ((TZXPulseToneLen = 3220) Or (TZXPulseToneLen = 8064)) And (TZXSync1Len = 667) And (TZXSync2Len = 735) And (TZXZeroLen = 855) And (TZXOneLen = 1710) Then TZXBlockIsStandardTiming = True Else TZXBlockIsStandardTiming = False
        
        Case &H12:  ' Pure Tone
            TZXState = 0 ' playing a possible pilot tone
            TZXPulseLen = TZXArray(TZXPointer + 1) + (TZXArray(TZXPointer + 2) * 256&)
            TZXPulseToneLen = TZXArray(TZXPointer + 3) + (TZXArray(TZXPointer + 4) * 256&)
            TZXAimTStates = TZXPulseLen
            TZXByte = 1
            TZXCurBlock = BlockNum
        
        Case &H13:  ' Row of Pulses
            TZXState = 0 ' playing a possible pilot tone
            TZXPulseToneLen = TZXArray(TZXPointer + 1) ' // NUMBER OF PULSES
            TZXPulseLen = TZXArray(TZXPointer + 2) + (TZXArray(TZXPointer + 3) * 256&)
            TZXPulsesDone = 1
            TZXByte = TZXPointer + 4
            TZXAimTStates = TZXPulseLen
            TZXCurBlock = BlockNum
        
        Case &H14:  ' Pure Data block
            TZXZeroLen = TZXArray(TZXPointer + 1) + (TZXArray(TZXPointer + 2) * 256&)
            TZXOneLen = TZXArray(TZXPointer + 3) + (TZXArray(TZXPointer + 4) * 256&)
            TZXUsedBits = TZXArray(TZXPointer + 5)
            TZXPauseLen = TZXArray(TZXPointer + 6) + (TZXArray(TZXPointer + 7) * 256&)
            TZXDataLen = TZXBlockLength(BlockNum) + TZXOffsets(BlockNum)
            TZXState = 3 ' Set to DATA Byte(s) output.
            ' // CC IN
            TZXDataPos = TZXPointer + 11
            ' // CC OUT
            TZXByte = TZXPointer + 11
            If (TZXArray(TZXByte) And 128) > 0 Then TZXAimTStates = TZXOneLen Else TZXAimTStates = TZXZeroLen
            TZXPulsesDone = 2
            If TZXByte = TZXDataLen - 1 Then TZXBitLimit = BitValues(8 - TZXUsedBits) Else TZXBitLimit = 1
            TZXBitCounter = 128
            TZXCurBlock = BlockNum
        
        Case &H15:  ' Direct Recording Block
            TZXOneLen = TZXArray(TZXPointer + 1) + (TZXArray(TZXPointer + 2) * 256&) ' Length of Sample (Ts)
            TZXPauseLen = TZXArray(TZXPointer + 3) + (TZXArray(TZXPointer + 4) * 256&) ' (ms)
            TZXUsedBits = TZXArray(TZXPointer + 5) ' Samples used in last byte
            TZXDataLen = TZXArray(TZXPointer + 6) + (TZXArray(TZXPointer + 7) * 256&) + TZXArray(TZXPointer + 8) * 65536 ' TZXBlockLength(BlockNum) + TZXOffsets(BlockNum)
            TZXByte = TZXPointer + 9
            TZXState = 3 ' Set to DATA bytes output
            TZXAimTStates = TZXOneLen
            If TZXByte = TZXDataLen - 1 Then TZXBitLimit = BitValues(8 - TZXUsedBits) Else TZXBitLimit = 1
            TZXBitCounter = 128
            glEarBit = 64 * (TZXArray(TZXByte) \ 128)
            TZXCurBlock = BlockNum
        
        Case &H20: ' Pause or STOP tape.
            TZXCurBlock = BlockNum
            TZXPauseLen = TZXArray(TZXPointer + 1) + (TZXArray(TZXPointer + 2) * 256&)
            If TZXPauseLen = 0 Then
                If gbTZXPlaying Then StartStopTape
            Else
                TZXAimTStates = TZXPauseLen * 3500
                TZXState = 4 ' When the TZXTStates gets past TZXAimStates, the next block will be used
            End If
        
        Case &H23:  ' Jump to block
            TZXByte = TZXArray(TZXPointer + 1) + (TZXArray(TZXPointer + 2) * 256&)
            If TZXByte < 32768 Then SetCurTZXBlock (BlockNum + TZXByte) Else SetCurTZXBlock (BlockNum - (65536 - TZXByte))
        
        Case &H24:  ' Loop Start
            TZXLoopCounter = TZXArray(TZXPointer + 1) + (TZXArray(TZXPointer + 2) * 256&)
            TZXLoopPoint = BlockNum + 1
            SetCurTZXBlock (BlockNum + 1)
        
        Case &H25:  ' Loop End
            TZXLoopCounter = TZXLoopCounter - 1
            If TZXLoopCounter > 0 Then SetCurTZXBlock (TZXLoopPoint) Else SetCurTZXBlock (BlockNum + 1)
        
        Case &H26:  ' Call Sequence
            TZXNumCalls = TZXArray(TZXPointer + 1) + (TZXArray(TZXPointer + 2) * 256&) - 1
            TZXCallByte = TZXNumCalls
            TZXCallCounter = 0
            ReDim TZXCallList(TZXNumCalls)
            For F = 0 To TZXNumCalls - 1
                TZXCallList(F) = TZXArray((TZXPointer + 4) + (F * 2)) + (TZXArray((TZXPointer + 5) + (F * 2) + 1) * 256&)
            Next F
            TZXCallByte = BlockNum
            TZXByte = TZXArray(TZXPointer + 3) + (TZXArray(TZXPointer + 4) * 256&)
            If TZXByte < 32768 Then SetCurTZXBlock (BlockNum + TZXByte) Else SetCurTZXBlock (BlockNum - (65536 - TZXByte))
        
        Case &H27:  ' CALL Return
            If TZXCallCounter < TZXNumCalls Then
                TZXCallCounter = TZXCallCounter + 1
                TZXByte = TZXCallList(TZXCallCounter)
                If TZXByte < 32768 Then SetCurTZXBlock (TZXCallByte + TZXByte) Else SetCurTZXBlock (TZXCallByte - (65536 - TZXByte))
            End If
        
        Case &H2A:  ' Stop tape in 48k Mode
            If glEmulatedModel = 0 Then ' 48k Speccy?
                 If gbTZXPlaying Then StartStopTape
            End If
            TZXCurBlock = BlockNum
        
        Case &HFE:  ' End of Tape
            TZXAimTStates = 30
            TZXCurBlock = BlockNum
            If gbTZXPlaying Then SetCurTZXBlock (0)
            StopTape
        
        Case Else: TZXCurBlock = BlockNum
    End Select
  
'    If TZXCurBlock > TZXLastDataBlock Then TZXState = 5
    If frmTapePlayer.Visible Then frmTapePlayer.UpdateCurBlock
End Sub

Public Sub UpdateTZXState(TapeTStates As Long)
    Dim LastEarBit As Long, F As Long
  
    TZXTotalTs = TZXTotalTs + TapeTStates
    While (TZXTotalTs >= TZXAimTStates) And gbTZXPlaying
        TZXTotalTs = TZXTotalTs - TZXAimTStates
        Select Case TZXCurBlockID
        Case &H10&, &H11&, &H14&
            Select Case TZXState
                Case 0 'Playing Pilot tone.
                    glEarBit = glEarBit Xor 64
                    If TZXByte < TZXPulseToneLen Then ' TZXByte holds number of pulses
                        TZXAimTStates = TZXPulseLen
                        TZXByte = TZXByte + 1&
                    Else
                        TZXByte = 0
                        TZXState = 1 ' Set to SYNC1 Pulse output
                        TZXAimTStates = TZXSync1Len
                    End If
                 
                 Case 1 ' SYNC 1
                    glEarBit = glEarBit Xor 64
                    TZXState = 2 ' Set to SYNC2 Pulse output
                    TZXAimTStates = TZXSync2Len
                 
                 Case 2 ' SYNC 2
                    glEarBit = glEarBit Xor 64
                    TZXState = 3 ' Set to DATA Byte(s) output
                    TZXByte = TZXDataPos
                    If (TZXArray(TZXByte) And 128) > 0 Then ' Set next pulse length
                        TZXAimTStates = TZXOneLen
                    Else
                        TZXAimTStates = TZXZeroLen
                    End If
                    TZXPulsesDone = 2 ' *2* edges per Data BIT, one on, one off
                    TZXBitCounter = 128 ' Start with the full byte
                    TZXBitLimit = 1
                 
                 Case 3 ' DATA Bytes out
                    glEarBit = glEarBit Xor 64
                    TZXPulsesDone = TZXPulsesDone - 1
                    If TZXPulsesDone = 0 Then ' Done both pulses for this bit?
                        If TZXBitCounter > TZXBitLimit Then ' Done all the bits for this byte?
                            TZXBitCounter = TZXBitCounter \ 2 ' Bitcounter counts *down*
                            TZXPulsesDone = 2
                            If (TZXArray(TZXByte) And TZXBitCounter) > 0 Then
                                TZXAimTStates = TZXOneLen
                            Else
                                TZXAimTStates = TZXZeroLen
                            End If
                        Else ' all bits done, setup for next byte
                            TZXByte = TZXByte + 1
                            If TZXByte < TZXDataLen Then ' last byte?
                                If TZXByte = TZXDataLen - 1 Then
                                    TZXBitLimit = BitValues(8 - TZXUsedBits) ' if so, set up the last bits used
                                Else
                                    TZXBitLimit = 1 ' else use full 8 bits
                                End If
                                TZXBitCounter = 128
                                TZXPulsesDone = 2
                                If (TZXArray(TZXByte) And 128) > 0 Then
                                    TZXAimTStates = TZXOneLen
                                Else
                                    TZXAimTStates = TZXZeroLen
                                End If
                            Else
                                If (TZXPauseLen > 0) Then
                                    TZXAimTStates = TZXPauseLen * 3500
                                    TZXState = 4 ' Set to Pause output
                                Else
                                    TZXState = 0
                                    SetCurTZXBlock (TZXCurBlock + 1)
                                End If
                            End If
                        End If
                    Else ' Not done both pulses, flip the ear bit next time
                        If (TZXArray(TZXByte) And TZXBitCounter) > 0 Then
                            TZXAimTStates = TZXOneLen
                        Else
                            TZXAimTStates = TZXZeroLen
                        End If
                    End If
                
                Case 4: ' End Pause output
                    SetCurTZXBlock (TZXCurBlock + 1)
            End Select
        
        Case &H12&
            glEarBit = glEarBit Xor 64
            If TZXByte < TZXPulseToneLen Then
                TZXAimTStates = TZXPulseLen
                TZXByte = TZXByte + 1
            Else
                SetCurTZXBlock (TZXCurBlock + 1)
            End If
        
        Case &H13&
            glEarBit = glEarBit Xor 64
            If TZXPulsesDone < TZXPulseToneLen Then
                TZXPulseLen = TZXArray(TZXByte) + (TZXArray(TZXByte + 1) * 256&)
                TZXAimTStates = TZXPulseLen
                TZXByte = TZXByte + 2
                TZXPulsesDone = TZXPulsesDone + 1
            Else
                SetCurTZXBlock (TZXCurBlock + 1)
            End If
        
        Case &H15& ' *UnTested* - any TZX actually use a DRB block?
            LastEarBit = glEarBit
            If TZXBitCounter > TZXBitLimit Then ' Done all the bits for this byte?
                TZXBitCounter = TZXBitCounter \ 2 ' Bitcounter counts *down*
                If (TZXArray(TZXByte) And TZXBitCounter) > 0 Then ' Set the ear bit
                    glEarBit = 64
                Else
                    glEarBit = 0
                End If
                TZXAimTStates = TZXOneLen
            Else ' all bits done, setup for next byte
                TZXByte = TZXByte + 1
                If TZXByte < TZXDataLen Then ' last byte?
                    If TZXByte = TZXDataLen - 1 Then
                        TZXBitLimit = BitValues(8 - TZXUsedBits) ' if so, set up the last bits used
                    Else
                       TZXBitLimit = 1 ' else use full 8 bits
                    End If
                    TZXBitCounter = 128
                    If (TZXArray(TZXByte) And TZXBitCounter) > 0 Then ' Set the ear bit
                        glEarBit = 64
                    Else
                        glEarBit = 0
                    End If
                    TZXAimTStates = TZXOneLen
                Else
                    If TZXPauseLen > 0 Then
                        TZXAimTStates = TZXPauseLen * 3500
                        TZXState = 4 ' Set to Pause output
                    Else
                        TZXState = 0
                        SetCurTZXBlock (TZXCurBlock + 1)
                    End If
                End If
            End If
        Case &HFE
            StopTape
        Case Else
            SetCurTZXBlock (TZXCurBlock + 1)
        End Select
    Wend
End Sub

