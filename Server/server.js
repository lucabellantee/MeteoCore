/*
  This script defines a web server using Express, configured to listen for HTTP POST requests on the `/api/data` route.

  The main functionalities include:
  - Enabling CORS to allow requests from other domains.
  - Parsing JSON requests using `body-parser` to extract data from the request body.
  - Sending a confirmation response with a JSON message.
*/

// Import the required libraries
const express = require('express');         
const bodyParser = require('body-parser'); 
const cors = require('cors');              

const app = express();     
const PORT = 3000;        

// Middleware to enable CORS, allowing HTTP calls from other domains (useful for separating frontend from backend)
app.use(cors());

// Middleware for parsing request bodies in JSON format
app.use(bodyParser.json());

// Define a POST route on /api/data that processes incoming requests
app.post('/api/data', (req, res) => {
  console.log('Data received:', req.body);
  
  // Send a confirmation response with a 200 status code (OK)
  res.status(200).send({ message: 'Data received successfully' });
});

// Configure the server to listen on the specified port and IP address
app.listen(PORT, '192.168.63.5', () => {
  console.log(`Server is up on http://192.168.63.5:${PORT}`);
});
