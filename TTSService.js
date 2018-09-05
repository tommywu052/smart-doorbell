/**
Copyright (c) Microsoft Corporation
All rights reserved. 
MIT License
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the ""Software""), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**/
var request = require('request'),
    xmlbuilder = require('xmlbuilder'),
    wav = require('wav'),
    Speaker = require('speaker');

var exec = require('child_process').exec;

exports.Synthesize = function Synthesize(mytts){

    // Note: new unified SpeechService API key and issue token uri is per region
    // New unified SpeechService key
    // Free: https://azure.microsoft.com/en-us/try/cognitive-services/?api=speech-services
    // Paid: https://go.microsoft.com/fwlink/?LinkId=872236
    var apiKey = "apikey";
    var ssml_doc = xmlbuilder.create('speak')
        .att('version', '1.0')
        .att('xml:lang', 'en-us')
        .ele('voice')
        .att('xml:lang', 'en-us')
        .att('xml:gender', 'Female')
        .att('name', 'Microsoft Server Speech Text to Speech Voice (zh-TW, Yating, Apollo)')
        .txt(mytts)
        .end();
    var post_speak_data = ssml_doc.toString();

   

    request.post({
        url: 'https://westus.api.cognitive.microsoft.com/sts/v1.0/issueToken',
        headers: {
            'Ocp-Apim-Subscription-Key' : apiKey
        }
    }, function (err, resp, access_token) {
        if (err || resp.statusCode != 200) {
            console.log(err, resp.body);
        } else {
            try {
                request.post({
                    url: 'https://westus.tts.speech.microsoft.com/cognitiveservices/v1',
                    body: post_speak_data,
                    headers: {
                        'content-type' : 'application/ssml+xml',
                        'X-Microsoft-OutputFormat' : 'riff-24khz-16bit-mono-pcm',
                        'Authorization': 'Bearer ' + access_token,
                        'X-Search-AppId': '07D3234E49CE426DAA29772419F436CA',
                        'X-Search-ClientID': '1ECFAE91408841A480F00935DC390960',
                        'User-Agent': 'TTSNodeJS'
                    },
                    encoding: null
                }, function (err, resp, speak_data) {
                    if (err || resp.statusCode != 200) {
                        console.log(err, resp.body);
                    } else {
                        try {
                            var reader = new wav.Reader();
                            reader.on('format', function (format) {
                                reader.pipe(new Speaker(format).on('open', function(evt){
                                    // handle playback error
                                    console.log('start play');
                                    exec('nircmd.exe mutesysvolume 1 microphone');

                                  }).on('close', function(evt){
                                    // handle playback error
                                    console.log('close play');
                                    exec('nircmd.exe mutesysvolume 0 microphone');
                                  }));
                            });
                            var Readable = require('stream').Readable;
                            var s = new Readable;
                            s.push(speak_data);
                            s.push(null);
                            s.pipe(reader);
                        } catch (e) {
                            console.log(e.message);
                        }
                    }
                });
            } catch (e) {
                console.log(e.message);
            }
        }
    });

    

};
