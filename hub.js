const { EventHubClient, EventPosition } = require('azure-event-hubs');
var SerialPort = require('serialport'); 
var Readline = SerialPort.parsers.Readline;
var serialPort = new SerialPort('/dev/ttyACM0', {
  baudRate: 9600
  //parser: new SerialPort.parsers.readline("\r")
});
"use strict";
const process = require('process');
const stream = require('stream');
const fileSystem = require('fs');
const bingSpeechService = require("ms-bing-speech-service");

var mic = require('mic');
var micInstance = mic({
  rate: '16000',
  bitwidth: '16',
  channels: '1',
  debug: false,
  exitOnSilence: 6
});
var micInputStream = micInstance.getAudioStream();
var outputFileStream = fileSystem.WriteStream('output.wav'); 
micInputStream.pipe(outputFileStream);



const FILENAME = "goodbye.wav";
const BING_SUBSCRIPTION_KEY = "key"
const BING_RECOGNITION_MODE = "interactive";
const BING_LANGUAGE = "zh-TW";
const BING_RESULT_FORMAT = "detailed";

var tts = require('./TTSService.js');  
const LUISClient = require("./luis_sdk");

const APPID = "appid";
const APPKEY = "appkey";

var LUISclient = LUISClient({
  appId: APPID,
  appKey: APPKEY,
  verbose: true
});

// Bing options
const options = {
    subscriptionKey: BING_SUBSCRIPTION_KEY,
    language: BING_LANGUAGE,
    mode: BING_RECOGNITION_MODE,
    format: BING_RESULT_FORMAT
};


async function main() {
    const onError = (err) => {
    console.log("An error occurred on the receiver ", err);
    };
  const client = await EventHubClient.createFromIotHubConnectionString("HostName=a9iothub.azure-devices.net;key");
  const hubInfo = await client.getHubRuntimeInformation();
  console.log(hubInfo);
  // Create recognizer
  const recognizer = new bingSpeechService(options);

  // Event handler
  const handleRecognition = (event) => {
    const status = event.RecognitionStatus;
    console.log(`${status}:  ${JSON.stringify(event)}`);
    if(status=='Success'){
    LUISclient.predict(event.NBest[0].Lexical, {
        //On success of prediction
        onSuccess: function (response) {
          printOnSuccess(response);
        },      
        //On failure of prediction
        onFailure: function (err) {
          console.error(err);
        }
      });
    }
  };
  const onMessage = (eventData) => {
    //console.log(eventData.body[0].Probability);
    var arydata = eventData.body;
    arydata.sort(compare);
    console.log('sorted====')
    console.log(arydata);
    if(arydata[0].Probability>0.5 && arydata[0].Tag=='Orange' ){
        console.log('open door');
        tts.Synthesize('你是柳橙，已授權-門栓即將開啟',micInstance,false); 
        micInstance.pause();
        serialPort.write('{"Name":'+arydata[0].Tag+',"Lock":"true","R":0,"G":255,"B":255}');
    }
    if(arydata[0].Probability>0.5 && arydata[0].Tag=='Apple' ){
        console.log('close door');
        serialPort.write('{"Name":'+arydata[0].Tag+',"Lock":"false","R":10,"G":10,"B":10}');
        tts.Synthesize('你是蘋果-未經授權-訪客',micInstance,true); 
        micInstance.resume();
        // Initialize the recognizer
        recognizer.start().then(() => {
          console.log("Ms speech api connected");
          recognizer.on('recognition', (event) => {handleRecognition(event)});
          recognizer.on('close', () => {console.log("Recognizer is closed.")});
          recognizer.on('error', (error) => {console.error(error)});

        // The file stream will be sent to the MS recognizer
        recognizer.sendStream(micInputStream);
          
        }).catch((error) => {
        console.error("Error while trying to start the recognizer.");
        console.error(error);
        process.exit(1);
        });
    }
    
    // for ( i in arydata ){
    //     console.log('-->' + i + '=' + arydata[i].Probability);
    // }
    const enqueuedTime = eventData.annotations["x-opt-enqueued-time"];
    console.log("Enqueued Time: ", enqueuedTime);
  };
 
  const receiveHandler = client.receive("1", onMessage, onError, { eventPosition: EventPosition.fromEnqueuedTime(Date.now()) });
  serialPort.on("open", function () {
    console.log('open');
    serialPort.on('data', function(data) {
      console.log(data.toString());
      
      //serialPort.write('{"Name":"Tommy","Lock":"false","R":255,"G":255,"B":255}');
    });
     
  });
 // await client.close();
}

function compare(a,b) {
    if (a.Probability < b.Probability)
      return 1;
    if (a.Probability > b.Probability)
      return -1;
    return 0;
  }

  var printOnSuccess = function (response) {
    console.log("Query: " + response.query);
    console.log("Top Intent: " + response.topScoringIntent.intent);
    if(response.topScoringIntent.intent=='greetings')
    {console.log('請問哪裡找');
    tts.Synthesize('請問哪裡找',micInstance,true); 
    }
    if(response.topScoringIntent.intent=='goodbye')
    {console.log('好的，我晚點在聯絡你');
    tts.Synthesize('好的，我晚點在聯絡你',micInstance,true); 
    }
    if(response.topScoringIntent.intent=='Ask')
    {console.log('哦，我現在正在忙，可以晚點在過來嗎？');
    tts.Synthesize('哦，我現在正在忙，可以晚點在過來嗎？',micInstance,true); 
    }

    console.log("Entities:");
    for (var i = 1; i <= response.entities.length; i++) {
      console.log(i + "- " + response.entities[i-1].entity);
    }
    if (typeof response.dialog !== "undefined" && response.dialog !== null) {
      console.log("Dialog Status: " + response.dialog.status);
      if(!response.dialog.isFinished()) {
        console.log("Dialog Parameter Name: " + response.dialog.parameterName);
        console.log("Dialog Prompt: " + response.dialog.prompt);
      }
    }
  };


  micInputStream.on('data', function(data) {
  //  console.log("Recieved Input Stream: " + data.length);
    
});
 
micInputStream.on('error', function(err) {
    cosole.log("Error in Input Stream: " + err);
});
 
micInputStream.on('startComplete', function() {
    console.log("Got SIGNAL startComplete");
    /* setTimeout(function() {
            micInstance.pause();
    }, 5000); */
});
    
micInputStream.on('stopComplete', function() {
    console.log("Got SIGNAL stopComplete");
});
    
micInputStream.on('pauseComplete', function() {
    console.log("Got SIGNAL pauseComplete");
    // setTimeout(function() {
    //     micInstance.resume();
    // }, 5000);
});
 
micInputStream.on('resumeComplete', function() {
    console.log("Got SIGNAL resumeComplete");
    // setTimeout(function() {
    //     micInstance.stop();
    // }, 5000);
});
 
micInputStream.on('silence', function() {
    console.log("Got SIGNAL silence");
});
 
micInputStream.on('processExitComplete', function() {
    console.log("Got SIGNAL processExitComplete");
});
 

/* setTimeout(function() {
         micInstance.pause();
     }, 2000); */
micInstance.start();
  

 
main().catch((err) => {
  console.log(err);
});
