GLYPH_P(trgt, ascii, x, y, fnt, color, sizeX, sizeY)
BEGIN
 LOCAL g := fnt[2, ascii];
 
 LOCAL xAdvance := BITAND(BITSR(g,32),255);
 
  IF BITAND(g,#FFFFFFFF)==0 THEN
    RETURN xAdvance * sizeX;
  END;
  
  LOCAL w, h, dX, dY, xx;
  
  LOCAL yAdvance := fnt[5];

  w:=BITAND(BITSR(g,16),255);
  h:=BITAND(BITSR(g,24),255);
 
  dX := BITAND(BITSR(g,40), 255);
  dY := BITAND(BITSR(g,48), 255) - 256;
 
  x := x + dX * sizeX;
  y := y + (yAdvance + dY) * sizeY;
  
  LOCAL bitmap := fnt[1];

  LOCAL offset := BITAND(g,65535);
  LOCAL bitPosition := BITAND(offset, 7) * 8;
  offset := BITSR(offset, 3) + 1;
  LOCAL bits := BITSR(bitmap[offset], bitPosition);
  
  REPEAT
    FOR xx FROM 1 TO w DO
      IF bitPosition == 64 THEN
        bitPosition := 0;
        offset := offset + 1;
        bits := bitmap[offset];
      END;
     
      IF BITAND(bits, 1) == 1 THEN
        IF sizeX == 1 AND sizeY == 1 THEN
          PIXON_P(trgt, x + xx,y, color);
        ELSE
          RECT_P(trgt, x + xx * sizeX, y, x + xx * sizeX + sizeX, y + sizeY, color);
        END;
      END;
      bitPosition := bitPosition + 1;
      bits := BITSR(bits, 1);
    END;
   
    y + sizeY ▶ y;
    h - 1 ▶ h;
  UNTIL h == 0;
  
  RETURN xAdvance * sizeX;
END;

EXPORT FONT_P(trgt, text, x, y, fnt, color, sizeX, sizeY)
BEGIN
  LOCAL i, l := ASC(text);
 
  FOR i := 1 TO SIZE(l) DO
    IF x >= 320 THEN BREAK; END;
    IF l[i] < fnt[3] OR l[i] > fnt[4] THEN CONTINUE; END;
    x := x + GLYPH_P(trgt, l[i] - fnt[3] + 1, x, y, fnt, color, sizeX, sizeY);
  END;
END;
