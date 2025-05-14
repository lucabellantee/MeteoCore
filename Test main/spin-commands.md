# Comandi Spin per l'analisi di modelli Promela

Questo documento descrive i comandi principali utilizzati per l'analisi di modelli Promela con lo strumento Spin tramite un container Docker.

## Comandi principali

### Buildare il container
```bash
docker build -t spin .
```
Questo comando crea un'immagine Docker chiamata "myspin" basata sul Dockerfile presente nella directory corrente. L'immagine contiene l'ambiente necessario per eseguire Spin, inclusi tutti i pacchetti e le dipendenze richieste.

### Avviare ed entrare dentro il container
```bash
docker run -it --rm -v "%cd%:/work" spin
```
Questo comando avvia un container basato sull'immagine "myspin", monta la directory corrente del sistema host nella directory "/work" all'interno del container, e fornisce un terminale interattivo. L'opzione `--rm` assicura che il container venga rimosso automaticamente quando viene chiuso.

### Generare il verificatore
```bash
spin -a test_main.pml
```
Questo comando analizza il modello Promela (test_main.pml) e genera diversi file sorgente C (pan.c, pan.h, ecc.) che implementano un verificatore specifico per il modello. Questo verificatore è personalizzato per le caratteristiche del modello analizzato.

### Compilare ed eseguire il verificatore
```bash
gcc -o pan pan.c
./pan
```
Il primo comando compila il file pan.c, generando un eseguibile chiamato "pan". Il secondo comando esegue il verificatore. Questo verificatore controlla automaticamente la presenza di deadlock, assertion violations e altri problemi nel modello. Se non vengono specificati parametri, esegue una verifica di base.

### Eseguire una verifica specifica per deadlock
```bash
./pan -d
```
Questo comando esegue il verificatore concentrandosi specificamente sulla ricerca di deadlock (stati finali non validi). L'opzione `-d` attiva la verifica dei deadlock e mostra dettagli su tutti i possibili stati e transizioni del modello.

## Comandi aggiuntivi utili

### Verifica di starvation (non-progress cycles)
```bash
gcc -DNP -o pan pan.c
./pan -l -f
```
Compila il verificatore con il flag `-DNP` e lo esegue per cercare cicli di non-progresso, che possono indicare situazioni di starvation (un processo non ottiene mai le risorse necessarie).


### Verifica delle proprietà LTL specifiche
```bash
spin -run -ltl deadlock_free test_main.pml
```
Verifica una specifica proprietà LTL definita nel modello (in questo caso, "deadlock_free").


### Analisi completa con salvataggio dei risultati
```bash
spin -a test_main.pml
gcc -DSAFETY -o pan pan.c
./pan -d -v > analysis_results.txt 2>&1
```
Esegue un'analisi completa e salva i risultati in un file di testo per revisione futura.
