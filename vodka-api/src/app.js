const express = require("express");
const twilio = require('twilio');
const app = express();
app.use(express.json());

const accountSid = "AC35f55d787a8a836461c3c932ca9ce61d";
const authToken = "5b6fcd2abc02fa9ec8522f352f173c31"; 
const client = new twilio(accountSid, authToken);

const OPTIONS = { timeZone: "America/El_Salvador", hour12: false };

// Test req
app.get("/", (_, res) => {
    res.status(200).send("Hello world!");
});

// Get current hour and day
app.get("/time", async (_, res) => {
  let svTime = new Date().toLocaleTimeString("en-US", OPTIONS);
  let svDay = new Date().getDay("en-US", OPTIONS);
  let svHour = svTime.split(":")[0];

  let response = svHour + "," + svDay;
  res.send(response);
});

// Send Whatsapp notification
app.post("/message/:type", async (req, res) => {  
  let type = parseInt(req.params.type);
  let msg = "";

  switch(type) {
    case 1:
      msg = "Tanque vacio!";
      break;
    case 2:
      msg = "Tanque a punto de vaciarse!";
    case 3:
      msg = "Tanque en capacidad media!";
    case 4:
      msg = "Tanque lleno!";
      break;
    default:
      msg = "Error";
  }

  // console.log(type, msg);
  // res.send(msg);
  client.messages
    .create({
      body: msg, // 'Hello from Node'
      to: 'whatsapp:+50370398239',
      from: 'whatsapp:+14155238886', // From a valid Twilio number
    })
    .then((message) => {
      console.log(message.sid);
      res.send(message.sid);
    });
});

module.exports = app;
