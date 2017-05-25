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
FILE_DIR_PIPE=pipe/
MAIN_EXE=pmanager
MAIN_FILE=pManager.c
CHILD_EXE=child
CHILD_FILE=child.c
FILE_TEST=input.txt
TESTO_TEST=pnew mattia\npspawn mattia 1\npspawn mattia_1 10\npspawn mattia_1_5 5\npspawn mattia_1_5_3 7\npspawn mattia_1_5_3_4 6\npspawn mattia_1_5_3_4_3 5\npspawn mattia_1_5_3_4_3_2 4\npspawn mattia_1_5_3_4_3_2_2 3\npspawn mattia_1_5_3_4_3_2_2_1 3\npspawn mattia_1_5_3_4_3_2_2_1_2 2\npspawn mattia_1_5_3_4_3_2_2_1_2_1 2\npspawn mattia_1_5_3_4_3_2_2_1_2_1_1 2\npspawn mattia_1_5_3_4_3_2_2_1_2_1_1_1 2\npnew federico\npnew mattia_1\npnew andrea\npspawn andrea 8\npspawn andrea_4 8\npnew progetoSistemiOperativi\npspawn federico 40\npnew federico\npnew federico\npnew mattia_1_5_2\npnew federico\npnew andrea
#TESTO_TEST=pnew ciao\npspawn ciao 10\npnew ciao
TMP_FILE=src/tmp

.PHONY: clean build assets test

help:
	@echo "$(TESTO)"

tmp:
	@rm -f $(TMP_FILE)
	@rm -rf $(FILE_DIR_PIPE)
	@echo "File di cache rimossi!"

clean:
	@[ -f "$(TMP_FILE)" ] && echo "pManager in esecuzione... In caso di errore eseguire make tmp!" || $(MAKE) checkClean --no-print-directory

build:
	@[ -f "$(TMP_FILE)" ] && echo "pManager in esecuzione... In caso di errore eseguire make tmp!" || $(MAKE) checkBuild --no-print-directory

assets:
	@[ -f "$(TMP_FILE)" ] && echo "pManager in esecuzione... In caso di errore eseguire make tmp!" || $(MAKE) checkAssets --no-print-directory

test:
	@[ -f "$(TMP_FILE)" ] && echo "pManager in esecuzione... In caso di errore eseguire make tmp!" || $(MAKE) checkTest --no-print-directory

checkClean:
	@rm -rf $(FILE_DIR_SAVE) $(FILE_DIR_OTHER)
	@echo "Eliminazione file completata..."

checkBuild: clean
	@mkdir $(FILE_DIR_SAVE)
	@gcc -o $(FILE_DIR_SAVE)$(MAIN_EXE) $(FILE_DIR_FIND)$(MAIN_FILE)
	@gcc -o $(FILE_DIR_SAVE)$(CHILD_EXE) $(FILE_DIR_FIND)$(CHILD_FILE)
	@echo "Compilazione file completata..."

checkAssets: build
	@mkdir $(FILE_DIR_OTHER)
	@echo "$(TESTO_TEST)" > $(FILE_DIR_OTHER)$(FILE_TEST)
	@echo "Creazione file di test completata..."

checkTest: assets
	@echo "Avvio shell passando file di test in corso...\n"
	@sleep 2
	@./$(FILE_DIR_SAVE)$(MAIN_EXE) $(FILE_DIR_OTHER)$(FILE_TEST)
