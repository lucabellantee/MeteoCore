/* 
 * Modello Promela per il sistema Zephyr BME280 & Rain Prediction
 * 
 * Questo modello simula il comportamento di un sistema embedded basato su Zephyr
 * che utilizza un sensore BME280 per acquisire dati ambientali (temperatura, pressione, umidità)
 * e un algoritmo di machine learning per prevedere la probabilità di pioggia.
 * Il sistema è composto da due thread principali che condividono un buffer protetto da mutex.
 */

/* 
 * Costanti riprese dal programma originale 
 * Definiscono i parametri fondamentali di configurazione del sistema
 */
#define MAX_SAMPLES 7               /* Numero massimo di campioni che possono essere memorizzati nel buffer */
#define SAMPLING_INTERVAL 10        /* Intervallo di campionamento in secondi */
#define PREDICTION_INTERVAL 61      /* Intervallo di previsione in secondi */

/* 
 * Canali per la comunicazione
 * Simulano la comunicazione con i dispositivi hardware (I2C per il sensore, UART per l'ESP32)
 */
chan i2c_channel = [1] of {byte};   /* Canale per la comunicazione I2C con il sensore BME280 */
chan uart_channel = [1] of {byte};  /* Canale per la comunicazione UART con l'ESP32 */

/* 
 * Mutex per la sincronizzazione
 * Protegge l'accesso al buffer condiviso tra i thread
 * false = sbloccato, true = bloccato
 */
bool buffer_mutex = false;          /* Inizialmente sbloccato */

/* 
 * Semaforo di inizializzazione
 * Usato per sincronizzare l'avvio dei thread dopo l'inizializzazione del sistema
 */
byte init_sem = 0;                  /* Inizializzato a 0, i thread attendono che sia > 0 */

/* 
 * Stato del buffer
 * Rappresenta il numero di campioni attualmente presenti nel buffer condiviso
 */
byte buffer_count = 0;              /* Inizialmente vuoto */

/* 
 * Stati dei dispositivi
 * Indicano se i vari componenti hardware sono pronti e inizializzati
 */
bool i2c_ready = true;              /* Stato del bus I2C */
bool uart_ready = true;             /* Stato dell'interfaccia UART */
bool bme280_initialized = true;     /* Stato di inizializzazione del sensore BME280 */

/* 
 * Flag di stato globali
 * Indicano lo stato di esecuzione dei vari componenti del sistema
 */
bool data_acquisition_running = false;  /* Stato del thread di acquisizione dati */
bool prediction_running = false;        /* Stato del thread di previsione */
bool ml_model_initialized = false;      /* Stato di inizializzazione del modello ML */
bool esp32_comm_initialized = false;    /* Stato della comunicazione con ESP32 */

/* 
 * Stati di errore
 * Rappresentano i possibili errori che possono verificarsi durante l'esecuzione
 */
bool i2c_error = false;             /* Errore di comunicazione I2C */
bool uart_error = false;            /* Errore di comunicazione UART */
bool sensor_read_error = false;     /* Errore di lettura del sensore */
bool esp32_send_error = false;      /* Errore di invio dati all'ESP32 */

/* 
 * Funzione inline per le operazioni di mutex
 * Implementa l'acquisizione atomica del mutex con attesa attiva
 */
inline lock_mutex() {
  atomic {
    /* Attende fino a quando il mutex è libero, poi lo blocca atomicamente */
    !buffer_mutex -> buffer_mutex = true;
  }
}

/* 
 * Funzione inline per rilasciare il mutex
 * Imposta semplicemente il flag del mutex a false (sbloccato)
 */
inline unlock_mutex() {
  buffer_mutex = false;
}

/* 
 * Funzione inline per acquisire un semaforo
 * Attende atomicamente che il valore del semaforo sia > 0, poi lo decrementa
 */
inline sem_take() {
  atomic {
    init_sem > 0 -> init_sem--;
  }
}

/* 
 * Funzione inline per rilasciare un semaforo
 * Incrementa semplicemente il valore del semaforo
 */
inline sem_give() {
  init_sem++;
}

/* 
 * Thread di acquisizione dati
 * Responsabile della lettura periodica dei dati dal sensore BME280
 * e dell'archiviazione nel buffer condiviso
 */
proctype data_acquisition_thread() {
  /* Imposta il flag che indica che il thread è in esecuzione */
  data_acquisition_running = true;
  
  /* Attende che l'inizializzazione sia completata (semaforo > 0) */
  sem_take();
  
  /* Loop infinito di acquisizione dati */
  do
  :: true -> /* Loop forever */
    /* Tenta di leggere i dati dal sensore */
    if
    :: !i2c_error -> /* Se non ci sono errori I2C */
      /* Acquisisce il mutex per accedere al buffer condiviso */
      lock_mutex();
      
      /* Aggiunge i dati al buffer se c'è spazio disponibile */
      if
      :: buffer_count < MAX_SAMPLES -> /* Se c'è spazio nel buffer */
        buffer_count++; /* Incrementa il contatore del buffer */
      :: else -> skip; /* Altrimenti non fa nulla */
      fi;
      
      /* Rilascia il mutex dopo aver aggiornato il buffer */
      unlock_mutex();
    :: else -> /* Se c'è un errore I2C */
      sensor_read_error = true; /* Imposta il flag di errore di lettura */
    fi;
    
    /* Simula l'attesa per il prossimo intervallo di campionamento */
    skip; /* Rappresenta k_sleep(K_MSEC(SAMPLING_INTERVAL_MS)) */
  od;
}

/* 
 * Thread di previsione
 * Responsabile dell'elaborazione dei dati acquisiti e dell'invio delle previsioni all'ESP32
 * Esegue a intervalli più lunghi rispetto al thread di acquisizione dati
 */
proctype prediction_thread() {
  /* Imposta il flag che indica che il thread è in esecuzione */
  prediction_running = true;
  
  /* Attende che l'inizializzazione sia completata (semaforo > 0) */
  sem_take();
  
  /* Loop infinito di previsione */
  do
  :: true -> /* Loop forever */
    /* Acquisisce il mutex per accedere al buffer condiviso */
    lock_mutex();
    
    /* Verifica se ci sono dati da elaborare */
    if
    :: buffer_count > 0 -> /* Se ci sono dati nel buffer */
      /* Azzera il buffer dopo aver utilizzato i dati (simula il calcolo della media) */
      buffer_count = 0;
      
      /* Tenta di inviare i dati elaborati all'ESP32 */
      if
      :: !uart_error -> skip; /* Se non ci sono errori UART, continua */
      :: else -> esp32_send_error = true; /* Altrimenti imposta il flag di errore */
      fi;
    :: else -> skip; /* Se non ci sono dati, non fa nulla */
    fi;
    
    /* Rilascia il mutex dopo aver elaborato i dati */
    unlock_mutex();
    
    /* Simula l'attesa per il prossimo intervallo di previsione */
    skip; /* Rappresenta k_sleep(K_MSEC(PREDICTION_INTERVAL_MS)) */
  od;
}

/* 
 * Processo principale
 * Responsabile dell'inizializzazione di tutti i componenti del sistema
 * e dell'avvio dei thread di acquisizione dati e previsione
 */
proctype main_process() {
  /* Verifica che il bus I2C sia pronto */
  if
  :: !i2c_ready -> /* Se I2C non è pronto */
    i2c_error = true; /* Imposta il flag di errore I2C */
    goto end; /* Salta alla fine senza completare l'inizializzazione */
  :: else -> skip; /* Altrimenti continua */
  fi;
  
  /* Verifica che l'interfaccia UART sia pronta */
  if
  :: !uart_ready -> /* Se UART non è pronto */
    uart_error = true; /* Imposta il flag di errore UART */
    goto end; /* Salta alla fine senza completare l'inizializzazione */
  :: else -> skip; /* Altrimenti continua */
  fi;
  
  /* Verifica che il sensore BME280 sia inizializzato */
  if
  :: !bme280_initialized -> /* Se il sensore non è inizializzato */
    goto end; /* Salta alla fine senza completare l'inizializzazione */
  :: else -> skip; /* Altrimenti continua */
  fi;
  
  /* Inizializza il modello di machine learning */
  ml_model_initialized = true;
  
  /* Inizializza la comunicazione con l'ESP32 */
  if
  :: true -> /* Caso normale: inizializzazione riuscita */
    esp32_comm_initialized = true;
  :: else -> /* Caso di errore: inizializzazione fallita */
    goto end; /* Salta alla fine senza completare l'inizializzazione */
  fi;
  
  /* Segnala che l'inizializzazione è completa rilasciando i semafori */
  sem_give(); /* Rilascia il primo semaforo (per il thread di acquisizione) */
  sem_give(); /* Rilascia il secondo semaforo (per il thread di previsione) */
  
  end: /* Etichetta per il salto in caso di errori */
  skip; /* Non fa nulla, termina il processo */
}

/* 
 * Proprietà di sicurezza (espresse in Linear Temporal Logic)
 * Verificano che il sistema soddisfi determinate proprietà durante tutta l'esecuzione
 */

/* 
 * deadlock_free: verifica che un mutex acquisito venga sempre rilasciato
 * [] (buffer_mutex -> <> !buffer_mutex) significa:
 * "Sempre ([]): se il mutex è acquisito (buffer_mutex), allora
 * eventualmente (<>) sarà rilasciato (!buffer_mutex)"
 */
ltl deadlock_free { [] (buffer_mutex -> <> !buffer_mutex) }

/* 
 * no_buffer_overflow: verifica che il buffer non superi mai la capacità massima
 * [] (buffer_count <= MAX_SAMPLES) significa:
 * "Sempre ([]): il conteggio del buffer è minore o uguale a MAX_SAMPLES"
 */
ltl no_buffer_overflow { [] (buffer_count <= MAX_SAMPLES) }

/* 
 * mutex_exclusion: verifica l'esclusione mutua corretta
 * [] !(data_acquisition_running && prediction_running && buffer_mutex) significa:
 * "Sempre ([]): non è mai vero che entrambi i thread sono in esecuzione e
 * contemporaneamente il mutex è in uno stato inconsistente"
 */
ltl mutex_exclusion { [] !(data_acquisition_running && prediction_running && buffer_mutex) }

/* 
 * initialization_property: verifica che entrambi i thread vengano avviati
 * <> (data_acquisition_running && prediction_running) significa:
 * "Eventualmente (<>): entrambi i thread saranno in esecuzione"
 */
ltl initialization_property { <> (data_acquisition_running && prediction_running) }

/* 
 * Processo di inizializzazione
 * Avvia tutti i processi definiti nel modello nell'ordine corretto
 */
init {
  run main_process();             /* Avvia il processo principale */
  run data_acquisition_thread();  /* Avvia il thread di acquisizione dati */
  run prediction_thread();        /* Avvia il thread di previsione */
}