const WebSocket = require('ws');

const wss = new WebSocket.Server({ port: 8765 });

let senderClient = null;
let receiverClient = null;

wss.on('connection', (ws) => {
  console.log('Client connected');

  ws.on('message', (message) => {
    try {
      const data = JSON.parse(message);

      if (data.type === 'sender') {
        senderClient = ws;
        console.log('Sender client connected');
        ws.send(JSON.stringify({ status: 'Sender registered' }));
      } else if (data.type === 'receiver') {
        receiverClient = ws;
        console.log('Receiver client connected');
        ws.send(JSON.stringify({ status: 'Receiver registered' }));
      } else if (senderClient === ws) {
        if (receiverClient) {
          receiverClient.send(JSON.stringify({ topic: data.topic, value: data.value }));
          console.log(`Forwarded to receiver: ${message}`);
        } else {
          console.log('No receiver client connected');
        }
      }
    } catch (error) {
      console.error('Error processing message:', error.message);
    }
  });

  ws.on('close', () => {
    console.log('Client disconnected');
    if (ws === senderClient) senderClient = null;
    if (ws === receiverClient) receiverClient = null;
  });
});

console.log('WebSocket server running on ws://localhost:8765');
