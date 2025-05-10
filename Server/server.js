const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');

const app = express();
const PORT = 3000;

app.use(cors());
app.use(bodyParser.json());

app.post('/api/data', (req, res) => {
  console.log('Dati ricevuti:', req.body);
  res.status(200).send({ message: 'Dati ricevuti correttamente' });
});

app.listen(PORT, '192.168.63.121', () => {
  console.log(`Server attivo su http://192.168.63.121:${PORT}`);
});

