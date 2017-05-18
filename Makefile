#Il Makefile contiene (oltre ad eventuali altre regole aggiuntive di appoggio che si ritengano utili) le regole:
#○ help (default): deve mostrare a video i componenti del gruppo (nome, cognome, matricola), una descrizione brevissima del progetto e le regole utilizzabili
#○ clean : deve cancellare la/e cartella/e build ed eventualmente assets se presente/i
#○ build : deve creare una cartella “build” con dentro il/i file(s) eseguibile/i richiamando prima clean
#○ assets (opzionale): deve creare una cartella “assets” con dentro il/i file(s) di supporto richiamando prima build (ad esempio per la modalità di “test”)
#○ test (opzionale): esegue il progetto in modalità “test” richiamando prima assets


TESTO=Mattia Milani\t\t178509\nFederico Brugiolo\t179294\nAndrea Fadi\t\t181494\n\nDESCRIZIONE PROGETTO: pManager, utilizzato per gestire dei processi.\n\nREGOLE UTILIZZABILI:\n\thelp (default): deve mostrare a video i componenti del gruppo (nome, cognome, matricola), una descrizione brevissima del progetto e le regole utilizzabili\n\tclean : deve cancellare la/e cartella/e build ed eventualmente assets se presente/i\n\tbuild : deve creare una cartella “build” con dentro il/i file(s) eseguibile/i richiamando prima clean\n\tassets (opzionale): deve creare una cartella “assets” con dentro il/i file(s) di supporto richiamando prima build (ad esempio per la modalità di “test”)\n\ttest (opzionale): esegue il progetto in modalità “test” richiamando prima assets\n

FILE_DIR_FIND=src/
FILE_DIR_SAVE=build/
FILE_DIR_OTHER=assets/
MAIN_EXE=pmanager
MAIN_FILE=pManager.c
CHILD_EXE=child
CHILD_FILE=child.c
FILE_TEST=input.txt
TESTO_TEST=pnew andreamattiafederico01\npnew andreamattiafederico02\npnew andreamattiafederico03\npspawn andreamattiafederico03 6\npspawn andreamattiafederico03_3 6\npspawn andreamattiafederico03_3_3 4\npspawn andreamattiafederico03_3_3_2 4\npspawn andreamattiafederico03_3_3_2 4\npspawn andreamattiafederico03_3_3_2_2 2\npspawn andreamattiafederico03_3_3_2_2_1 2\npspawn andreamattiafederico03_3_3_2_2_1_1\npnew andreamattiafederico04\npnew andreamattiafederico05\npnew andreamattiafederico06\npspawn andreamattiafederico06 6\npspawn andreamattiafederico06_3 6\npspawn andreamattiafederico06_3_3 4\npspawn andreamattiafederico06_3_3_2 4\npspawn andreamattiafederico06_3_3_2_2 4\npspawn andreamattiafederico06_3_3_2_2_2 2\npspawn andreamattiafederico06_3_3_2_2_2_1 2\npspawn andreamattiafederico06_3_3_2_2_2_1_1\npnew andreamattiafederico07\npnew andreamattiafederico08\npnew andreamattiafederico09
TMP_FILE=src/tmp

help:
	@echo "$(TESTO)"

clean:
	@rm -rf $(FILE_DIR_SAVE) $(FILE_DIR_OTHER)
	@echo "Eliminazione file completata..."

build: clean
	@mkdir $(FILE_DIR_SAVE)
	@gcc -o $(FILE_DIR_SAVE)$(MAIN_EXE) $(FILE_DIR_FIND)$(MAIN_FILE)
	@gcc -o $(FILE_DIR_SAVE)$(CHILD_EXE) $(FILE_DIR_FIND)$(CHILD_FILE)
	@echo "Compilazione file completata..."
	
assets: build
	@mkdir $(FILE_DIR_OTHER)
	@echo "$(TESTO_TEST)" > $(FILE_DIR_OTHER)$(FILE_TEST)
	@echo "Creazione file di test completata..."

exist:
	@test -f "$(TMP_FILE)" || $(MAKE) assets

test: exist
	@./$(FILE_DIR_SAVE)$(MAIN_EXE) $(FILE_DIR_OTHER)$(FILE_TEST)
