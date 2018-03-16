PROGRAM ASCzuKAT;

TYPE Block   = ARRAY[0..511] OF Byte;

VAR KatName,AscName : String;
    KatF            : File OF Block;
    AscF            : Text;

(* konvertiert Ascii-Dateien in KAT-Ce Texte *)

(*$I-*)

PROCEDURE Konvertiere;

VAR  KatBlock       : Block;
     i,Zeiger       : Integer;
     L              : Longint;
     ASCZeile       : String[255];

  PROCEDURE PutKat(b: Byte; VAR i: Integer; VAR l: LongInt);
  BEGIN
    KatBlock[i]:=b;
    Inc(i); inc(l);
    IF i=512 THEN BEGIN Write(KatF,Katblock); i:=0 END
  END;

BEGIN
  i:=0; l:=0;
  PutKat(0,i,l);PutKat(0,i,l);PutKat(0,i,l); PutKat(0,i,l); (* Startadr *)
  PutKat(0,i,l);PutKat(0,i,l);PutKat(0,i,l); PutKat(0,i,l); (* Laenge   *)
  WHILE NOT eof(AscF) DO
    BEGIN readln(ascF,AscZeile); writeln(AscZeile);
          PutKat($10,i,l);
          Zeiger:=1;
          IF Length(AscZeile)>0
             THEN WHILE AscZeile[Zeiger]=' ' DO inc(Zeiger);
          PutKat(Zeiger-1+$20,i,l);
          WHILE Zeiger<=Length(AscZeile)
            DO BEGIN PutKat(Ord(AscZeile[Zeiger]),i,l); Inc(Zeiger) END;
          PutKat($0D,i,l);
    END;
  PutKat($10,i,l); PutKat($20,i,l); PutKat($00,i,l);
  IF i<> 0 THEN Write(KatF,KatBlock);
  Reset(KatF);    (* auf ersten Block zeigen *)
  Read(KatF,KatBlock);
  KatBlock[7]:=L AND $FF;
  KatBlock[6]:=(L shr 8) AND $FF;
  KatBlock[5]:=(L shr 16) AND $FF;
  KatBlock[4]:=(L shr 24);
  reset(KatF);
  write(KatF,KatBlock)
END;



BEGIN
  writeln; writeln;
  write('Welche ASCII-Datei konvertieren: '); readln(ASCname);
  Assign(AscF,AscName); Reset(AscF);
  IF Ioresult <> 0
    THEN BEGIN writeln('Datei nicht vorhanden'); readln; halt END;
  write('Name der neu zu erstellenden KAT-Ce-Datei: '); readln(KatName);
  assign(KatF,KatName); Rewrite(KatF);
  IF IOResult <> 0
    THEN BEGIN writeln('Datei kann nicht erzeugt werden',#7,#7); readln; halt END;
  Konvertiere;
  Close(KatF); Close(AscF)
END.


