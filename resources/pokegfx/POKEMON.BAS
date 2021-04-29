DECLARE FUNCTION hexx! (text$)
DECLARE FUNCTION LongToString$ (L&)
DECLARE SUB savebmp8 (Pic$, X1%, Y1%, X2%, Y2%)
DECLARE SUB SetPalette (Attr%, red%, green%, blue%)
DECLARE SUB GetPalette (Attr%, red%, green%, blue%)
DECLARE FUNCTION bin2dec! (numba$)
DECLARE FUNCTION Hex2Bin$ (Hcs$)
DIM tu AS STRING
DIM tu2 AS STRING
DIM tu3 AS STRING
DIM tubina AS INTEGER
DIM SHARED tt(1 TO 100) AS INTEGER

DIM SHARED red(0 TO 255) AS INTEGER
DIM SHARED green(0 TO 255) AS INTEGER
DIM SHARED blue(0 TO 255) AS INTEGER

DIM yloop AS INTEGER
DIM tpos AS LONG


DIM SHARED bmp(1 TO 100, 1 TO 100) AS INTEGER


'OPEN "pokeg.gbc" FOR BINARY AS #1
'OPEN "dunno.dat" FOR OUTPUT AS #2
tu = " "
tu2 = " "
tu3 = " "
kloop = 0
mloop = 0
mpal = 0
SCREEN 13

pokem = 5

DO
'FOR pokem = 1 TO 251
pokstart:
LOCATE 19, 1: COLOR 2: PRINT "0 to quit."
LOCATE 17, 1: COLOR 2: INPUT "Pokemon number?", pokem
IF pokem = 201 THEN PRINT "Can't show Unown.": GOTO pokstart
IF pokem = 0 THEN EXIT DO
IF pokem < 1 OR pokem > 251 THEN PRINT "Invalid number.": GOTO pokstart
OPEN "pokeg.gbc" FOR BINARY AS #1
tpos = 294912 + (6 * (pokem - 1)) + 1
tpospal = 44349 + (8 * (pokem)) + 1
'savefile$ = STR$(pokem) + "g.bmp"
GOSUB printpoke
kloop = 70
tpos = 294912 + (6 * (pokem - 1)) + 4
tpospal = 44349 + (8 * (pokem)) + 1
'savefile$ = STR$(pokem) + "gs.bmp"
GOSUB printpoke
kloop = 0: mloop = 70: mpal = 4
tpos = 294912 + (6 * (pokem - 1)) + 1
tpospal = 44349 + (8 * (pokem)) + 5
'savefile$ = STR$(pokem) + "gb.bmp"
GOSUB printpoke
kloop = 70
tpos = 294912 + (6 * (pokem - 1)) + 4
tpospal = 44349 + (8 * (pokem)) + 5
'savefile$ = STR$(pokem) + "gbs.bmp"
GOSUB printpoke
kloop = 0: mloop = 140: mpal = 0
CLOSE #1

OPEN "pokes.gbc" FOR BINARY AS #1
tpos = 294912 + (6 * (pokem - 1)) + 1
tpospal = 44349 + (8 * (pokem)) + 1
'savefile$ = STR$(pokem) + "s.bmp"
GOSUB printpoke
kloop = 70
tpos = 294912 + (6 * (pokem - 1)) + 4
tpospal = 44349 + (8 * (pokem)) + 1
'savefile$ = STR$(pokem) + "ss.bmp"
GOSUB printpoke
kloop = 0: mloop = 210: mpal = 4
tpos = 294912 + (6 * (pokem - 1)) + 1
tpospal = 44349 + (8 * (pokem)) + 5
'savefile$ = STR$(pokem) + "sb.bmp"
GOSUB printpoke
kloop = 70
tpos = 294912 + (6 * (pokem - 1)) + 4
tpospal = 44349 + (8 * (pokem)) + 5
'savefile$ = STR$(pokem) + "sbs.bmp"
GOSUB printpoke
kloop = 0: mloop = 0: mpal = 0
CLOSE #1

LOOP
'NEXT pokem
END


printpoke:

IF (kloop = 0 AND mloop = 0 AND mpal = 0) THEN CLS
final$ = ""
ERASE bmp

badpal:
GET #1, tpos, tu
GET #1, tpos + 1, tu2
GET #1, tpos + 2, tu3
kl = ASC(tu)
IF kl = 19 THEN
 kl = 31
ELSEIF kl = 20 THEN
 kl = 32
ELSEIF kl = 31 THEN
 kl = 46
END IF
tpos = (kl - 1) * 16384 + hexx(RIGHT$("00" + HEX$(ASC(tu3)), 2) + RIGHT$("00" + HEX$(ASC(tu2)), 2)) + 1


DO
GET #1, tpos, tu
tpos = tpos + 1
tubina = ASC(tu)
low5 = (ASC(tu) AND 31)
IF ASC(tu) = 255 THEN
 EXIT DO
END IF

doroutine:
IF (tubina AND 224) = 0 THEN    '00
 FOR k = 0 TO low5
 GET #1, tpos, tu
 tpos = tpos + 1
 final$ = final$ + tu
 NEXT k
ELSEIF (tubina AND 224) = 32 THEN    '20
GET #1, tpos, tu
tpos = tpos + 1
FOR k = 0 TO low5
final$ = final$ + tu
NEXT k

ELSEIF (tubina AND 224) = 64 THEN    '40
GET #1, tpos, tu
GET #1, tpos + 1, tu2
tpos = tpos + 2
FOR k = 0 TO low5
final$ = final$ + tu
k = k + 1
IF k > low5 THEN EXIT FOR
final$ = final$ + tu2
NEXT k

ELSEIF (tubina AND 224) = 96 THEN    '60
FOR k = 0 TO low5
final$ = final$ + CHR$(0)
NEXT k

ELSEIF (tubina AND 224) = 128 THEN    '80
GET #1, tpos, tu
tpos = tpos + 1
IF (ASC(tu) AND 128) = 0 THEN
carryamt = ASC(tu)
 GET #1, tpos, tu
 tpos = tpos + 1

FOR k = 0 TO low5
 temp$ = MID$(final$, (ASC(tu) + 1) + (carryamt * 256) + k, 1)
 final$ = final$ + temp$
NEXT k

ELSE
 sly = (ASC(tu) AND 127)

FOR k = 0 TO low5
 temp$ = MID$(final$, LEN(final$) - sly, 1)
 final$ = final$ + temp$
NEXT k


END IF

ELSEIF (tubina AND 224) = 160 THEN    'A0
GET #1, tpos, tu
tpos = tpos + 1
IF (ASC(tu) AND 128) = 0 THEN
carryamt = ASC(tu)
 GET #1, tpos, tu
 tpos = tpos + 1
FOR x = 0 TO low5
 temp$ = Hex2Bin$(RIGHT$("00" + HEX$(ASC(MID$(final$, ASC(tu) + x + 1 + (carryamt * 256), 1))), 2))
 temp2$ = ""
 FOR k = 1 TO 8
 temp2$ = temp2$ + MID$(temp$, 9 - k, 1)
 NEXT k
 final$ = final$ + CHR$(bin2dec(temp2$))
NEXT x
ELSE
 sly = (ASC(tu) AND 127)

klen = LEN(final$)
FOR x = 0 TO low5
 temp$ = Hex2Bin$(RIGHT$("00" + HEX$(ASC(MID$(final$, klen - sly + x, 1))), 2))
 temp2$ = ""
 FOR k = 1 TO 8
 temp2$ = temp2$ + MID$(temp$, 9 - k, 1)
 NEXT k
 final$ = final$ + CHR$(bin2dec(temp2$))
NEXT x
END IF

ELSEIF (tubina AND 224) = 192 THEN    'C0
GET #1, tpos, tu
tpos = tpos + 1
IF (ASC(tu) AND 128) = 0 THEN
carryamt = ASC(tu)
GET #1, tpos, tu
 tpos = tpos + 1
FOR k = 0 TO low5
 temp$ = MID$(final$, (ASC(tu) + 1) + (carryamt * 256) - k, 1)
 final$ = final$ + temp$
NEXT k
ELSE
 sly = (ASC(tu) AND 127)
klen = LEN(final$)
FOR k = 0 TO low5
 temp$ = MID$(final$, klen - sly - k, 1)
 final$ = final$ + temp$
NEXT k
END IF


ELSEIF (tubina AND 224) = 224 THEN    'E0
tubina = (tubina AND 28) * 8
GET #1, tpos, tu
tpos = tpos + 1
low5 = (low5 AND 3) * 256 + ASC(tu)
GOTO doroutine
END IF

LOOP
final$ = final$
'PRINT #2, final$;

IF ((LEN(final$) <> 400) AND (LEN(final$)) <> 576) AND LEN(final$) <> 784 THEN
lh = 5
END IF
xloop = (LEN(final$) / 16) ^ .5
yloop = xloop * 8


FOR x = 1 TO xloop
FOR y = 1 TO yloop

k$ = Hex2Bin$(RIGHT$("00" + HEX$(ASC(MID$(final$, 2 * yloop * (x - 1) + 2 * (y - 1) + 1, 1))), 2))
k2$ = Hex2Bin$(RIGHT$("00" + HEX$(ASC(MID$(final$, 2 * yloop * (x - 1) + 2 * (y - 1) + 2, 1))), 2))
FOR bmpbit = 1 TO 8
bmp((x - 1) * 8 + bmpbit, y) = bin2dec(MID$(k2$, bmpbit, 1) + MID$(k$, bmpbit, 1))
NEXT bmpbit
NEXT y
NEXT x


GET #1, tpospal, tu
GET #1, tpospal + 1, tu2
rgb$ = Hex2Bin$(RIGHT$("00" + HEX$(ASC(tu2)), 2) + RIGHT$("00" + HEX$(ASC(tu)), 2))
b% = bin2dec(MID$(rgb$, 2, 5) + "1")
g% = bin2dec(MID$(rgb$, 7, 5) + "1")
r% = bin2dec(MID$(rgb$, 12, 5) + "1")
red(1) = r%: green(1) = g%: blue(1) = b%
CALL SetPalette(1 + mpal, r%, g%, b%)

GET #1, tpospal + 2, tu
GET #1, tpospal + 3, tu2
rgb$ = Hex2Bin$(RIGHT$("00" + HEX$(ASC(tu2)), 2) + RIGHT$("00" + HEX$(ASC(tu)), 2))
b% = bin2dec(MID$(rgb$, 2, 5) + "1")
g% = bin2dec(MID$(rgb$, 7, 5) + "1")
r% = bin2dec(MID$(rgb$, 12, 5) + "1")

red(2) = r%: green(2) = g%: blue(2) = b%
CALL SetPalette(2 + mpal, r%, g%, b%)

red(0) = 63: green(0) = 63: blue(0) = 63
red(3) = 0: green(3) = 0: blue(3) = 0
CALL SetPalette(0 + mpal, 63, 63, 63)
CALL SetPalette(3 + mpal, 0, 0, 0)

'CALL savebmp8(savefile$, 1, 1, yloop, yloop)


FOR y = 1 TO yloop
FOR x = 1 TO yloop

PSET (x + mloop, y + kloop), bmp(x, y) + mpal
NEXT x
NEXT y

'IF kloop > 0 THEN
'WHILE INKEY$ = ""
'WEND
'END IF

RETURN

CLOSE #1
'CLOSE #2

FUNCTION bin2dec (numba$)
temp$ = "01"
numba$ = UCASE$(numba$)
length% = LEN(numba$)
FOR i% = length% TO 1 STEP -1
  char$ = MID$(numba$, i%, 1)
  posn% = INSTR(temp$, char$) - 1
  J% = length% - i%
  value& = value& + (posn% * (LEN(temp$) ^ J%))
NEXT i%
bin2dec = value&

END FUNCTION

DEFINT A-Z
SUB GetPalette (Attr%, red%, green%, blue%)

OUT &H3C7, Attr%
red% = INP(&H3C9)
green% = INP(&H3C9)
blue% = INP(&H3C9)

END SUB

DEFSNG A-Z
FUNCTION Hex2Bin$ (Hcs$)

Hcs$ = UCASE$(Hcs$)
lc = LEN(Hcs$)
FOR x = 1 TO lc
 SELECT CASE MID$(Hcs$, x, 1)
  CASE "0"
   Out$ = Out$ + "0000"
  CASE "1"
   Out$ = Out$ + "0001"
  CASE "2"
   Out$ = Out$ + "0010"
  CASE "3"
   Out$ = Out$ + "0011"
  CASE "4"
   Out$ = Out$ + "0100"
  CASE "5"
   Out$ = Out$ + "0101"
  CASE "6"
   Out$ = Out$ + "0110"
  CASE "7"
   Out$ = Out$ + "0111"
  CASE "8"
   Out$ = Out$ + "1000"
  CASE "9"
   Out$ = Out$ + "1001"
  CASE "A"
   Out$ = Out$ + "1010"
  CASE "B"
   Out$ = Out$ + "1011"
  CASE "C"
   Out$ = Out$ + "1100"
  CASE "D"
   Out$ = Out$ + "1101"
  CASE "E"
   Out$ = Out$ + "1110"
  CASE "F"
   Out$ = Out$ + "1111"
 END SELECT
NEXT
Hex2Bin$ = Out$
END FUNCTION

FUNCTION hexx (text$)

i = 1
temp1$ = ""
temp2$ = ""
final = 0
gh = LEN(text$)
DO WHILE i < gh + 1
temp1$ = MID$(text$, i, 1)

IF ASC(temp1$) > 47 AND ASC(temp1$) < 58 THEN
 tt(i) = VAL(temp1$)
ELSEIF UCASE$(temp1$) = "A" THEN
  tt(i) = 10
ELSEIF UCASE$(temp1$) = "B" THEN
  tt(i) = 11
ELSEIF UCASE$(temp1$) = "C" THEN
  tt(i) = 12
ELSEIF UCASE$(temp1$) = "D" THEN
  tt(i) = 13
ELSEIF UCASE$(temp1$) = "E" THEN
  tt(i) = 14
ELSEIF UCASE$(temp1$) = "F" THEN
  tt(i) = 15
ELSE
  tt(i) = -1: gh = i - 1: GOTO 23
END IF
i = i + 1
LOOP
23
i = 1
k = 1
FOR i = 1 TO gh
IF tt(i) = -1 THEN GOTO 34
final = final + tt(i) * (16 ^ (gh - i))

NEXT i
34
hexx = final

END FUNCTION

DEFINT A-Z
FUNCTION LongToString$ (L&)

B4% = L& \ 16777216
L& = L& - B4% * 16777216
B3% = L& \ 65536
L& = L& - B3% * 65536
B2% = L& \ 256
L& = L& - B2% * 256
B1% = L&
LongToString$ = CHR$(B1%) + CHR$(B2%) + CHR$(B3%) + CHR$(B4%)

END FUNCTION

SUB savebmp8 (Pic$, X1%, Y1%, X2%, Y2%)

'Calculate width and height of image-
BMPWidth% = X2% - X1% + 1
BMPHeight% = Y2% - Y1% + 1
'Each raster must be a multiple of 4 bytes, this next line takes
'care of 'padded' bytes at the end of rasters of odd-width images-
IF BMPWidth% / 4 <> BMPWidth% \ 4 THEN PadBytes% = 4 - (BMPWidth% MOD 4)
OPEN Pic$ FOR BINARY AS #3
'General Picture Information-
'  BMP format marker-
Buffer$ = "BM"
'  File size minus header-
L& = (BMPWidth% + PadBytes%) * BMPHeight% + 1078
Buffer$ = Buffer$ + LongToString$(L&)
'  Reserved 1-
Buffer$ = Buffer$ + CHR$(0) + CHR$(0)
'  Reserved 2-
Buffer$ = Buffer$ + CHR$(0) + CHR$(0)
'  Number of bytes offset to picture data-
Buffer$ = Buffer$ + LongToString$(1078)
'Information Header-
'  Size of information header-
Buffer$ = Buffer$ + LongToString$(40)
'  Picture width in pixels-
L& = BMPWidth%
Buffer$ = Buffer$ + LongToString$(L&)
'  Picture height in pixels-
L& = BMPHeight%
Buffer$ = Buffer$ + LongToString$(L&)
'  Number of planes-
Buffer$ = Buffer$ + CHR$(1) + CHR$(0)
'  Bits per pixel-
Buffer$ = Buffer$ + CHR$(8) + CHR$(0)
'  Compression-
Buffer$ = Buffer$ + LongToString$(0)
'  Image size in bytes-
L& = (BMPWidth% + PadBytes%) * BMPHeight%
Buffer$ = Buffer$ + LongToString$(L&)
'  Picture width in pixels per meter-
Buffer$ = Buffer$ + LongToString$(0)
'  Picture height in pixels per meter-
Buffer$ = Buffer$ + LongToString$(0)
'  Colors used in picture-
Buffer$ = Buffer$ + LongToString$(256)
'  Number of important colors-
Buffer$ = Buffer$ + LongToString$(256)
PUT #3, 1, Buffer$
'Save palette data-
Buffer$ = ""
FOR i% = 0 TO 255
'  GetPalette i%, Red%, Green%, Blue%
  'Palette is saved B, G, R with unused byte trailing-
  Buffer$ = Buffer$ + CHR$(blue(i%) * 4 + 3)
  Buffer$ = Buffer$ + CHR$(green(i%) * 4 + 3)
  Buffer$ = Buffer$ + CHR$(red(i%) * 4 + 3)
  Buffer$ = Buffer$ + CHR$(0)
NEXT i%
PUT #3, , Buffer$
'Save image data-


FOR i% = (BMPHeight% - 1) TO 0 STEP -1
  Buffer$ = ""
  FOR J% = 0 TO (BMPWidth% - 1)
    Buffer$ = Buffer$ + CHR$(bmp(X1% + J%, Y1% + i%))
  NEXT J%
  IF PadBytes% > 0 THEN
    FOR J% = 1 TO PadBytes%
      Buffer$ = Buffer$ + CHR$(0)
    NEXT J%
  END IF
  PUT #3, , Buffer$
NEXT i%



'Put a fork in it, it's done-
CLOSE #3

END SUB

SUB SetPalette (Attr%, red%, green%, blue%)

OUT &H3C7, Attr%
OUT &H3C8, Attr%
OUT &H3C9, red%
OUT &H3C9, green%
OUT &H3C9, blue%

END SUB

