PROGRAM KATzuASC;

TYPE Block   = ARRAY[0..127] OF Byte;

VAR KatName,AscName: String;
    KatF: File OF Block;
    AscF: Text;

(* konvertiert KAT-Ce Texte in ASCII-Dateien *)

(*$I-*)

PROCEDURE Konvertiere;

VAR  KatBlock       : Block;
     i,Sp           : Integer;
     Ende,DLE,Start : Boolean;
     ASCZeile       : String[255];
     KatChr         : Byte;
BEGIN
  Ende:=Eof(KatF);
  DLE:=False; ASCZeile:=''; Start:=True;
  WHILE NOT Ende DO
    BEGIN read(KatF,KatBlock);
          IF Start THEN i:=8 ELSE i:=0; Start:=False;
          WHILE (NOT Ende) AND (i <= 127) DO
            BEGIN KatChr:=KatBlock[i];
                  IF DLE
                     THEN BEGIN FOR Sp:=$20 TO KatChr
                                    DO ASCZeile:=ASCZeile+' ';
                                DLE:=False;
                          END
                     ELSE CASE KatChr OF
                            $10: DLE:=True;
                            $00: Ende:=True;
                            $0D: BEGIN writeln(AscF,ASCZeile);
                                       writeln(ASCZeile);
                                       ASCZeile:=''
                                 END;
                            ELSE ASCZeile:=ASCZeile+Chr(KatChr)
                          END;
                  i:=i+1;
            END;
            Ende:=Ende OR eof(KatF)
    END
END;



BEGIN
  writeln; writeln;
  write('Welche KAT-Ce Datei konvertieren: '); readln(Katname);
  Assign(KatF,Katname); Reset(KatF);
  IF Ioresult <> 0
    THEN BEGIN writeln('Datei nicht vorhanden'); readln; halt END;
  write('Name der neu zu erstellenden ASCII-Datei: '); readln(AscName);
  assign(AscF,AscName); Rewrite(AscF);
  IF IOResult <> 0
    THEN BEGIN writeln('Datei kann nicht erzeugt werden',#7,#7); readln; halt END;
  Konvertiere;
  Close(KatF); Close(AscF)
END.


