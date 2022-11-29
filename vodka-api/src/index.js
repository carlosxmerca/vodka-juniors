const http = require("http");
const app = require("./app");
const colors = require('colors');
const server = http.createServer(app);

const port = process.env.PORT || 3000;

// server listening 
server.listen(port, () => {
  console.log(`Server running on port ${port}`.blue);
});
