// Scan for networks when the page loads
window.addEventListener('load', getScannedNetworks);

document.getElementById("ssid").onclick = getScannedNetworks;
// Function to get scanned networks
function getScannedNetworks(){
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        var myObj = JSON.parse(this.responseText);
        console.log(myObj);
        createNetworkOptions(myObj);
      }
    };
    xhr.open("GET", "/scan", true);
    xhr.send();
}

function createNetworkOptions(jsonNetworks) {

    var select = document.getElementById('ssid');
    var keys = Object.keys(jsonNetworks);
    // console.log(keys);
    // console.log(keys.length);

    if (keys.length < 1)
      return;

    while (select.hasChildNodes())
      select.removeChild(select.firstChild);

    var empty_opt = document.createElement('option');
    select.appendChild(empty_opt);

    for (var i = 0; i < keys.length; i++){
        var opt = document.createElement('option');
        opt.value = jsonNetworks[keys[i]].ssid;
        opt.innerHTML = jsonNetworks[keys[i]].ssid;
        select.appendChild(opt);
    }

}