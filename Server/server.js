/*
  Questo script definisce un server web utilizzando, configurato per ascoltare richieste HTTP POST su una rotta `/api/data`.

  Le principali funzionalità includono:
  - Abilitazione di CORS per consentire richieste da altri domini.
  - Parsing delle richieste JSON tramite `body-parser` per estrarre i dati dal corpo della richiesta.
  - Risposta di conferma con un messaggio JSON.
*/

// Importa le librerie necessarie
const express = require('express');         
const bodyParser = require('body-parser'); 
const cors = require('cors');              

const app = express();     
const PORT = 3000;        

// Middleware per abilitare CORS, permettendo chiamate HTTP da altri domini (utile per separare il frontend dal backend)
app.use(cors());

// Middleware per il parsing del corpo delle richieste in formato JSON
app.use(bodyParser.json());

// Definizione di una rotta POST su /api/data, che processa le richieste in arrivo
app.post('/api/data', (req, res) => {
  console.log('Dati ricevuti:', req.body);
  
  // Invia una risposta di conferma con un codice di stato 200 (OK)
  res.status(200).send({ message: 'Dati ricevuti correttamente' });
});

// Configura il server per ascoltare le richieste sulla porta specificata e su un indirizzo IP specifico
app.listen(PORT, '192.168.63.121', () => {
  console.log(`Server attivo su http://192.168.63.121:${PORT}`);
});


