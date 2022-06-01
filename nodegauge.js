const dgram = require('dgram');
const struct = require('python-struct');
const client = dgram.createSocket('udp4');
const express = require('express');
const path = require('path');
const app = express();
const port = 80;

time = "1"
car = "bea"
flags = '49152'
gear = '1'
speed = 0
rpm = 0
turbo = 0
engtemp = 90
fuel = 1
oilpressure = 0
oiltemp = 70
dashlights = 1894
showlights = 0
throttle = 0
brake = 1
clutch = 0
display1 = ''
display2 = ''



setInterval(()=>{
	let packedmessage = struct.pack('f3sxHB8fI3fsxsx', time, car, flags, gear, speed, rpm, turbo, engtemp, fuel, oilpressure, oiltemp, dashlights, showlights, throttle, brake, clutch,display1,display2);
	console.log('send: ', time, car, flags, gear, speed, rpm, turbo, engtemp, fuel, oilpressure, oiltemp, dashlights, showlights, throttle, brake, clutch, display1, display2);
	client.send(packedmessage, 0, packedmessage.length, 4444, '10.0.0.88');
}, 0.005);


/*
setInterval(()=>{
	rpm+=100;
	if(rpm==6000) { rpm = 0; speed +=0.27777777777/6; }
}, 1000);
*/


app.get('/speed', (req, res) => {
  speed = ((req.query.speed));
  res.send('_')
})

app.get('/rpm', (req, res) => {
  rpm = parseInt((req.query.rpm));
  res.send('_')
})

app.get('/fuel', (req, res) => {
  fuel = (parseInt(req.query.fuel)/100);
  res.send('_')
})

app.get('/temp', (req, res) => {
  oiltemp = (parseInt(req.query.temp));
  res.send('_')
})

let abs = false;
let left = false;
let right = false;
let ebrake = false;
let beam = false;
let warning = false;
let fldoor = false;
let frdoor = false;
let rrdoor = false;
let rldoor = false;
let hood = false;
let kr = false;

app.get('/dashlights', (req, res) => {
  showlights = 0;

  if(req.query.abs == 'true') abs = true; 
  if(req.query.left == 'true') left = true;
  if(req.query.right == 'true') right = true;
  if(req.query.ebrake == 'true') ebrake = true;
  if(req.query.beam == 'true') beam = true; 
  if(req.query.warning == 'true') warning = true; 
  if(req.query.fldoor == 'true') fldoor = true; 
  if(req.query.frdoor == 'true') frdoor = true; 
  if(req.query.rrdoor == 'true') rrdoor = true; 
  if(req.query.rldoor == 'true') rldoor = true; 
  if(req.query.hood == 'true') hood = true; 
  if(req.query.kr == 'true') kr = true; 
  
  if(req.query.abs == 'false') abs = false; 
  if(req.query.left == 'false') left = false;
  if(req.query.right == 'false') right = false;
  if(req.query.ebrake == 'false') ebrake = false;
  if(req.query.beam == 'false') beam = false; 
  if(req.query.warning == 'false') warning = false; 
  if(req.query.fldoor == 'false') fldoor = false; 
  if(req.query.frdoor == 'false') frdoor = false; 
  if(req.query.rrdoor == 'false') rrdoor = false; 
  if(req.query.rldoor == 'false') rldoor = false; 
  if(req.query.hood == 'false') hood = false; 
  if(req.query.kr == 'false') kr = false; 
  
  if(abs) showlights+= 1024;
  if(ebrake) showlights+= 4;
  if(beam) showlights+= 2;
  if(right) showlights+= 64;
  if(left) showlights+= 32;
  if(warning) showlights+= 32+64;
  if(fldoor) showlights+= 2048;
  if(frdoor) showlights+= 4096;
  if(rrdoor) showlights+= 8192;
  if(rldoor) showlights+= 16384;
  if(hood) showlights+= 32768;
  if(kr) showlights+= 65536;
  
  res.send('_')
})


app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, '/html.html'));
})

app.listen(port, () => {
  console.log(`Example app listening on port ${port}`)
})

