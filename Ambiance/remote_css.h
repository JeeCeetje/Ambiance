const char remote_css[] PROGMEM = R"=====(
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    background-color: #eeeeee;
    font-family:  Arial;
}

.remote-container {
    padding: 10px;
    display: flex;
    justify-content: center;
    font-weight: bold;
}

.remote {
    border: 5px solid #000000;
    background: #222222;
    width: 310px;
}

.remote-switch {
    color : #000000;
}

@media only screen and (max-width: 400px) {
    .remote {
        width: 100%;
    }
}

.header,
.footer {
    padding: 1%;
    text-align: center;
    color: #ffffff
}

.button-row {
    margin: 0% 2%;
    display: flex;
    justify-content: space-around;
}

.button-row-wheel {
    margin: 4% -3%;
    display: flex;
    justify-content: space-around;
}

.button {
    background: #ffffff;
    margin: 2%;
    width: 20%;
    border-radius: 50%;
    text-align: center;
}
.button:before {
    content: "";
    display: inline-block;
    padding: 50% 0;
    vertical-align: middle;
}

.noselect {
    -webkit-touch-callout: none;
      -webkit-user-select: none;
       -khtml-user-select: none;
         -moz-user-select: none;
          -ms-user-select: none;
              user-select: none;
}
)=====";
