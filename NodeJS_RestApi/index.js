const express = require("express");
const app = express();
const port = 3000;

app.use(express.json());

app.get("/", function (req, res) {
  res.send("Hello World!");
});

app.get("/data", function (req, res) {
  dummyData = {
    firstName: "Max",
    lastName: "Mustermann",
  };
  res.status(200).type("json").send(dummyData);
});

app.post("/echo", function (req, res) {
  console.log("Post request with following body: ");
  console.log(req.body);


});

app.listen(port, () =>
  console.log(`Hello world app listening on port ${port}!`)
);
