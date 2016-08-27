
// execute an API call to the ESP8266 server
function send_cmd(oper, cmd, chan, eid) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
	// if response is uknown (due to unsuccessful request) do no update the eid element
        if (xhttp.readyState == 4 && xhttp.status == 200) {
	    console.debug("response: [" + xhttp.responseText.substring(0,2) + "]");
	    eid.checked = (xhttp.responseText.substring(0,2) == 'on' ? true : false);
        }
    };

    //xhttp.open("GET","/chan?"+cmd, true);
    //xhttp.send();

    xhttp.open("POST", "/oper", true);
    xhttp.setRequestHeader("Content-type", "application/json");
    xhttp.send(JSON.stringify( { "oper":oper,"chan": chan,"cmd": cmd } ));
}

// toggle the specified channel state
function toggle_channel(chan, eid) {
    if (eid.checked) {
	send_cmd('set', 'on', chan, eid);
    }
    else {
	send_cmd('set', 'off', chan, eid)
    }
}

// after application page is fully loaded execute a first api call to
// get the status of the channels
document.addEventListener('DOMContentLoaded', function() {
    cb1 = document.getElementById("led1");
    document.getElementById("led1").addEventListener("click", function(event) {
	cb1 = this;
	toggle_channel(1, cb1);
	event.preventDefault();
    });
    send_cmd('get', 'status', 1, cb1);

});

