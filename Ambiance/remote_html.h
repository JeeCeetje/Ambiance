const char remote_html[] PROGMEM = R"=====(
<!doctype html>
<html lang='en'>
    <head>
        <meta charset='utf-8'/>
        <meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0'/>
        <title></title>
        <link rel='stylesheet' href='remote.css'/>
        <link rel='icon' href='favicon.ico' type='image/x-icon'/>
        <link rel='apple-touch-icon-precomposed' sizes='192x192' href='favicon.png'/>
        <script src='remote.js'></script>
    </head>
    <body>
        <div class='remote-container'>
            <div id='remote' class='remote'></div>
        </div>
        <div class='remote-container'>
            <div id='remote-switch' class='remote-switch '></div>
        </div>
    </body>
</html>
)=====";
