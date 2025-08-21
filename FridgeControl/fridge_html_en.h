const char FRIDGE_page_en[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>

<meta HTTP-EQUIV="Content-Type" CONTENT="text/html; charset=UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">

<style>

h1 {
  font-size: 1.5em;
  text-align: center; 
  vertical-align: middle; 
  max-width: 400px; 
  margin:0 auto;
}

p {
  font-size: 1.5em;
  text-align: center; 
  vertical-align: middle; 
  max-width: 400px; 
  margin:0 auto;
}

table {
  font-size: 1.5em;
  text-align: left; 
  vertical-align: middle; 
  margin:0 auto;
}

.button {
  font-size: 18px;;
  text-align: center; 
}

.slidecontainer {
  width: 100%;
}

.slider {
  -webkit-appearance: none;
  width: 90%;
  height: 24px;
  background: #d3d3d3;
  outline: none;
  opacity: 0.7;
  -webkit-transition: .2s;
  transition: opacity .2s;
}

.slider:hover {
  opacity: 1;
}

.slider::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 22px;
  height: 22px;
  background: #4CAF50;
  cursor: pointer;
}

.slider::-moz-range-thumb {
  width: 22px;
  height: 22px;
  background: #4CAF50;
  cursor: pointer;
}

</style>


<title>Fridge Control</title>
<hr>
<h1>Fridge Control</h1>
<hr>
</head>

<body style="font-family: verdana,sans-serif" BGCOLOR="#819FF7">
  <table>
    <tr><td style="text-align:right;">Temperature:</td><td style="color:white;width:150px;"> <span id='temp'></span> °C</td><tr>
    <tr><td style="text-align:right;">Average:</td><td style="color:white;width:150px;"> <span id='avg'></span> °C</td><tr>
    <tr><td style="text-align:right;">Set Temp.:</td><td style="color:white;width:150px;"> <span id='level'></span> °C</td><tr>
    <tr><td style="text-align:right;">Duty cycle:</td><td style="color:white;width:150px;"> <span id='dutycycle'></span></td><tr>
    <tr><td style="text-align:right;">Status:</td><td style="color:white;width:150px;"> <span id='status'></span></td><tr>
  </table>  
  <hr>  
  <div class="slidecontainer">
    <p><input type="range" min="0" max="10" value="6" step="0.1" class="slider" id="myRange"></p>
  </div>
  <p> 
  <input type="button" class="button" value=" - " onclick="button_clicked('f_down')"> 
  <input type="button" class="button" value=" + " onclick="button_clicked('f_up')">   
  </p>
  <br>
  <p> 
  <input type="button" class="button" value="Auto" onclick="button_clicked('f_auto')">
  <input type="button" class="button" value=" On " onclick="button_clicked('f_on')">
  <input type="button" class="button" value=" Off " onclick="button_clicked('f_off')"> 
  </p>
  <p>
  <input type="button" class="button" value="Boost" onclick="button_clicked('f_boost')">
  <input type="button" class="button" value="Defrost" onclick="button_clicked('f_defrost')">
  <input type="button" class="button" value=" Set " onclick="window.location.replace('/f_settings')"> 
  </p>

   
  <script>
  
    requestData(); // get intial data straight away 

    var slider = document.getElementById("myRange");
    var output = document.getElementById("level");
        
    slider.oninput = function() {
      output.innerHTML = this.value;
      var xhr = new XMLHttpRequest();
      xhr.open('GET', 'f_slider' + "?level=" + this.value, true);
      xhr.send();
    }

    function button_clicked(key) { 
      var xhr = new XMLHttpRequest();
      xhr.open('GET', key, true);
      xhr.send();
      requestData();
    }
  
    // request data updates every 500 milliseconds
    setInterval(requestData, 500);

    function requestData() {

      var xhr = new XMLHttpRequest();
      
      xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {

          if (this.responseText) { // if the returned data is not null, update the values

            var data = JSON.parse(this.responseText);

            document.getElementById("temp").innerText = data.temp;
            document.getElementById("status").innerText = data.status;
            document.getElementById("avg").innerText = data.avg;
            document.getElementById("dutycycle").innerText = data.dutycycle;
            
            output.innerHTML = data.level;
            slider.value = 1* data.level;
       
          } 
        } 
      }
      xhr.open('GET', 'f_temp', true);
      xhr.send();
    }
     
  </script>

</body>

</html>

)=====";


//-------------------------------------------------------------------------

const char Settings_page_en[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>

<meta HTTP-EQUIV="Content-Type" CONTENT="text/html; charset=UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">

<style>

h1 {
  font-size: 1.5em;
  text-align: center; 
  vertical-align: middle; 
  max-width: 400px; 
  margin:0 auto;
}

p {
  font-size: 1.5em;
  text-align: center; 
  vertical-align: middle; 
  max-width: 400px; 
  margin:0 auto;
}

table {
  font-size: 1.5em;
  text-align: left; 
  vertical-align: middle; 
  margin:0 auto;
}

.button {
  font-size: 18px;;
  text-align: center; 
}

.slidecontainer {
  width: 100%;
}

.slider {
  -webkit-appearance: none;
  width: 90%;
  height: 24px;
  background: #d3d3d3;
  outline: none;
  opacity: 0.7;
  -webkit-transition: .2s;
  transition: opacity .2s;
}

.slider:hover {
  opacity: 1;
}

.slider::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 22px;
  height: 22px;
  background: #4CAF50;
  cursor: pointer;
}

.slider::-moz-range-thumb {
  width: 22px;
  height: 22px;
  background: #4CAF50;
  cursor: pointer;
}

</style>

<title>Settings</title>
<hr>
<h1>Settings</h1>
<hr>

</head>

<body style="font-family: verdana,sans-serif" BGCOLOR="#819FF7">

  <table>
    <tr><td style="text-align:right;">Language:</td><td style="color:white;"><input type="text" style="width:50px; font-size: 18px;" id="language"> 0=EN 1=DE</td></tr>
    <tr><td style="text-align:right;">Hysteresis:</td><td style="color:white;"><input type="text" style="width:50px; font-size: 18px;" id="hyst"> 1-5</td></tr>
    <tr><td style="text-align:right;">RPM:</td><td style="color:white;"><input type="text" style="width:50px; font-size: 18px;" id="rpm"> 0,>=2000<=3500</td></tr>
    <tr><td style="text-align:right;">AVG Error:</td><td style="color:white;"><input type="text" style="width:50px; font-size: 18px;" id="avgerror"> 0-2</td></tr>
    <tr><td style="text-align:right;">Max High:</td><td style="color:white;"><input type="text" style="width:50px; font-size: 18px;" id="maxhigh"> 5-12</td></tr>
    <tr><td style="text-align:right;">Max Low:</td><td style="color:white;"><input type="text" style="width:50px; font-size: 18px;" id="maxlow"> 0-5</td></tr>
    <tr><td style="text-align:right;">Boost Temp.:</td><td style="color:white;"><input type="text" style="width:50px; font-size: 18px;" id="boosttemp"> 1-6</td></tr>
    <tr><td style="text-align:right;">Boost Hyst.:</td><td style="color:white;"><input type="text" style="width:50px; font-size: 18px;" id="boosthyst"> 0.5-2</td></tr>
    <tr><td style="text-align:right;">Boost Time:</td><td style="color:white;"><input type="text" style="width:50px; font-size: 18px;" id="boosttime"> 10-120</td></tr>
    <tr><td style="text-align:right;">Boost RPM:</td><td style="color:white;"><input type="text" style="width:50px; font-size: 18px;" id="boostrpm"> >=2000<=3500</td></tr> 
    <tr><td style="text-align:right;">Max Runtime:</td><td style="color:white;"><input type="text" style="width:50px; font-size: 18px;" id="maxruntime"> 10-60</td></tr> 
    <tr><td style="text-align:right;">Defrost Time:</td><td style="color:white;"><input type="text" style="width:50px; font-size: 18px;" id="defrosttime"> 10-60</td></tr> 
  </table>
   
  <hr>
  <p>
  <input type="button" class="button" value="Back" onclick="window.location.replace('/')"> 
  <input type="button" class="button" value="Set" onclick="button_clicked_set()"> 
  </p>
    
  <script>

    requestData(); // get intial data 
    
    var language = document.getElementById("language");
    var hyst = document.getElementById("hyst");
    var rpm = document.getElementById("rpm");
    var avgerror = document.getElementById("avgerror"); 
    var maxhigh = document.getElementById("maxhigh"); 
    var maxlow = document.getElementById("maxlow"); 
    var boosttime = document.getElementById("boosttime");
    var boosttemp = document.getElementById("boosttemp"); 
    var boosthyst = document.getElementById("boosthyst"); 
    var boostrpm = document.getElementById("boostrpm"); 
    var maxruntime = document.getElementById("maxruntime"); 
    var defrosttime = document.getElementById("defrosttime"); 
    
    function button_clicked_set() { 
      var xhr = new XMLHttpRequest();
      
      xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
           window.location.replace("/f_settings");
        } 
      }
      xhr.open('GET', 'f_set_settings'+'?hyst='+hyst.value+'&rpm='+rpm.value+'&avgerror='+avgerror.value+
                      '&maxlow='+maxlow.value+'&maxhigh='+maxhigh.value +'&boosttime='+boosttime.value+
                      '&boostrpm='+boostrpm.value+'&boosttemp='+boosttemp.value+'&maxruntime='+maxruntime.value+
                      '&defrosttime='+defrosttime.value+'&boosthyst='+boosthyst.value+'&language='+language.value, true);
      xhr.send();   
    }
  
        
    function requestData() {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', 'f_get_settings');

      xhr.onload = function() {
        if (xhr.status === 200) {

          if (xhr.responseText) { // if the returned data is not null, update the values

            var data = JSON.parse(xhr.responseText);
                        
            language.value = data.language;
            hyst.value = data.hyst;
            rpm.value = data.rpm;
            avgerror.value = data.avgerror;
            maxhigh.value = data.maxhigh;
            maxlow.value = data.maxlow;
            boosttime.value = data.boosttime; 
            boosttemp.value = data.boosttemp;   
            boosthyst.value = data.boosthyst;   
            boostrpm.value = data.boostrpm;
            maxruntime.value = data.maxruntime;
            defrosttime.value = data.defrosttime;         
          } 
        } 
      }      
      xhr.send();
    }
     
  </script>

</body>

</html>

)=====";

