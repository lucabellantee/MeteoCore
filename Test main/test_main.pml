/* 
* Promela Model for Zephyr BME280 & Rain Prediction System
* 
* This model simulates the behavior of a Zephyr-based embedded system
* that uses a BME280 sensor to acquire environmental data (temperature, pressure, humidity)
* and a machine learning algorithm to predict rain probability.
* The system consists of two main threads that share a mutex-protected buffer.
*/

/* 
* Constants taken from the original program 
* Define the fundamental system configuration parameters
*/
#define MAX_SAMPLES 7               /* Maximum number of samples that can be stored in the buffer */
#define SAMPLING_INTERVAL 10        /* Sampling interval in seconds */
#define PREDICTION_INTERVAL 61      /* Prediction interval in seconds */

/* 
* Communication channels
* Simulate communication with hardware devices (I2C for sensor, UART for ESP32)
*/
chan i2c_channel = [1] of {byte};   /* Channel for I2C communication with BME280 sensor */
chan uart_channel = [1] of {byte};  /* Channel for UART communication with ESP32 */

/* 
* Mutex for synchronization
* Protects access to the shared buffer between threads
* false = unlocked, true = locked
*/
bool buffer_mutex = false;          /* Initially unlocked */

/* 
* Initialization semaphore
* Used to synchronize thread startup after system initialization
*/
byte init_sem = 0;                  /* Initialized to 0, threads wait until it's > 0 */

/* 
* Buffer state
* Represents the number of samples currently present in the shared buffer
*/
byte buffer_count = 0;              /* Initially empty */

/* 
* Device states
* Indicate whether various hardware components are ready and initialized
*/
bool i2c_ready = true;              /* I2C bus state */
bool uart_ready = true;             /* UART interface state */
bool bme280_initialized = true;     /* BME280 sensor initialization state */

/* 
* Global status flags
* Indicate the execution state of various system components
*/
bool data_acquisition_running = false;  /* Data acquisition thread state */
bool prediction_running = false;        /* Prediction thread state */
bool ml_model_initialized = false;      /* ML model initialization state */
bool esp32_comm_initialized = false;    /* ESP32 communication state */

/* 
* Error states
* Represent possible errors that can occur during execution
*/
bool i2c_error = false;             /* I2C communication error */
bool uart_error = false;            /* UART communication error */
bool sensor_read_error = false;     /* Sensor read error */
bool esp32_send_error = false;      /* ESP32 data send error */

/* 
* Additional variables to track thread access to critical section
* Used to verify mutual exclusion and fairness properties
*/
bool data_acquisition_in_critical = false;  /* Data acquisition thread in critical section */
bool prediction_in_critical = false;        /* Prediction thread in critical section */
bool data_acquisition_waiting = false;      /* Data acquisition thread waiting for mutex */
bool prediction_waiting = false;            /* Prediction thread waiting for mutex */

/* 
* Inline function for mutex operations
* Implements atomic mutex acquisition with busy waiting
*/
inline lock_mutex(thread_id) {
 atomic {
   /* Set waiting flag for the thread */
   if
   :: thread_id == 1 -> data_acquisition_waiting = true;
   :: thread_id == 2 -> prediction_waiting = true;
   fi;
   
   /* Wait until mutex is free, then lock it atomically */
   !buffer_mutex -> buffer_mutex = true;
   
   /* Remove waiting flag and set critical section flag */
   if
   :: thread_id == 1 -> 
     data_acquisition_waiting = false;
     data_acquisition_in_critical = true;
   :: thread_id == 2 -> 
     prediction_waiting = false;
     prediction_in_critical = true;
   fi;
 }
}

/* 
* Inline function to release mutex
* Simply sets the mutex flag to false (unlocked)
*/
inline unlock_mutex(thread_id) {
 atomic {
   buffer_mutex = false;
   /* Remove critical section flag */
   if
   :: thread_id == 1 -> data_acquisition_in_critical = false;
   :: thread_id == 2 -> prediction_in_critical = false;
   fi;
 }
}

/* 
* Inline function to acquire a semaphore
* Atomically waits for semaphore value > 0, then decrements it
*/
inline sem_take() {
 atomic {
   init_sem > 0 -> init_sem--;
 }
}

/* 
* Inline function to release a semaphore
* Simply increments the semaphore value
*/
inline sem_give() {
 init_sem++;
}

/* 
* Data acquisition thread
* Responsible for periodic reading of data from BME280 sensor
* and storing it in the shared buffer
*/
proctype data_acquisition_thread() {
 /* Set flag indicating thread is running */
 data_acquisition_running = true;
 
 /* Wait for initialization to complete (semaphore > 0) */
 sem_take();
 
 /* Infinite data acquisition loop */
 do
 :: true -> /* Loop forever */
   /* Attempt to read data from sensor */
   if
   :: !i2c_error -> /* If no I2C errors */
     /* Acquire mutex to access shared buffer */
     lock_mutex(1);
     
     /* Add data to buffer if space is available */
     if
     :: buffer_count < MAX_SAMPLES -> /* If there's space in buffer */
       buffer_count++; /* Increment buffer counter */
     :: else -> skip; /* Otherwise do nothing */
     fi;
     
     /* Release mutex after updating buffer */
     unlock_mutex(1);
   :: else -> /* If there's an I2C error */
     sensor_read_error = true; /* Set read error flag */
   fi;
   
   /* Simulate waiting for next sampling interval */
   skip; /* Represents k_sleep(K_MSEC(SAMPLING_INTERVAL_MS)) */
 od;
}

/* 
* Prediction thread
* Responsible for processing acquired data and sending predictions to ESP32
* Executes at longer intervals than the data acquisition thread
*/
proctype prediction_thread() {
 /* Set flag indicating thread is running */
 prediction_running = true;
 
 /* Wait for initialization to complete (semaphore > 0) */
 sem_take();
 
 /* Infinite prediction loop */
 do
 :: true -> /* Loop forever */
   /* Acquire mutex to access shared buffer */
   lock_mutex(2);
   
   /* Check if there's data to process */
   if
   :: buffer_count > 0 -> /* If there's data in buffer */
     /* Clear buffer after using data (simulates average calculation) */
     buffer_count = 0;
     
     /* Attempt to send processed data to ESP32 */
     if
     :: !uart_error -> skip; /* If no UART errors, continue */
     :: else -> esp32_send_error = true; /* Otherwise set error flag */
     fi;
   :: else -> skip; /* If no data, do nothing */
   fi;
   
   /* Release mutex after processing data */
   unlock_mutex(2);
   
   /* Simulate waiting for next prediction interval */
   skip; /* Represents k_sleep(K_MSEC(PREDICTION_INTERVAL_MS)) */
 od;
}

/* 
* Main process
* Responsible for initializing all system components
* and starting data acquisition and prediction threads
*/
proctype main_process() {
 /* Check if I2C bus is ready */
 if
 :: !i2c_ready -> /* If I2C is not ready */
   i2c_error = true; /* Set I2C error flag */
   goto end; /* Jump to end without completing initialization */
 :: else -> skip; /* Otherwise continue */
 fi;
 
 /* Check if UART interface is ready */
 if
 :: !uart_ready -> /* If UART is not ready */
   uart_error = true; /* Set UART error flag */
   goto end; /* Jump to end without completing initialization */
 :: else -> skip; /* Otherwise continue */
 fi;
 
 /* Check if BME280 sensor is initialized */
 if
 :: !bme280_initialized -> /* If sensor is not initialized */
   goto end; /* Jump to end without completing initialization */
 :: else -> skip; /* Otherwise continue */
 fi;
 
 /* Initialize machine learning model */
 ml_model_initialized = true;
 
 /* Initialize ESP32 communication */
 if
 :: true -> /* Normal case: initialization successful */
   esp32_comm_initialized = true;
 :: else -> /* Error case: initialization failed */
   goto end; /* Jump to end without completing initialization */
 fi;
 
 /* Signal initialization complete by releasing semaphores */
 sem_give(); /* Release first semaphore (for acquisition thread) */
 sem_give(); /* Release second semaphore (for prediction thread) */
 
 end: /* Label for jump in case of errors */
 skip; /* Do nothing, terminate process */
}

/* 
* Safety properties (expressed in Linear Temporal Logic)
* Verify that the system satisfies certain properties throughout execution
*/

/* 
* no_buffer_overflow: verifies that buffer never exceeds maximum capacity
* [] (buffer_count <= MAX_SAMPLES) means:
* "Always ([]): buffer count is less than or equal to MAX_SAMPLES"
*/
ltl no_buffer_overflow { [] (buffer_count <= MAX_SAMPLES) }

/* 
* Deadlock-free property between prediction and data acquisition threads
*
* This property verifies that there's never a situation where
* both threads are blocked waiting for each other, causing
* a system deadlock.
*/
ltl thread_deadlock_free { 
   [] (
       /* If prediction thread is waiting for mutex */
       (prediction_running && !buffer_mutex) -> 
       /* Then it will eventually get the mutex */
       <> (buffer_mutex)
   ) && 
   [] (
       /* If data acquisition thread is waiting for mutex */
       (data_acquisition_running && !buffer_mutex) -> 
       /* Then it will eventually get the mutex */
       <> (buffer_mutex)
   )
}



/*
* Mutual exclusion property
* 
* This property verifies that the two threads never access
* the critical section (shared buffer) simultaneously.
* At any moment, at most one thread can be in the critical section.
*/
ltl mutual_exclusion {
   [] !(data_acquisition_in_critical && prediction_in_critical)
}

/* 
* Initialization process
* Starts all processes defined in the model in the correct order
*/
init {
 run main_process();             /* Start main process */
 run data_acquisition_thread();  /* Start data acquisition thread */
 run prediction_thread();        /* Start prediction thread */
}