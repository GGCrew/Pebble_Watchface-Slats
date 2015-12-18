var SERVER_NAME = "net-night.com";
var APP_NAME = "slats";


Pebble.addEventListener("ready",
    function(e) {
        console.log("Hello world! - Sent from your javascript application.");
    }
);


Pebble.addEventListener('showConfiguration', 
	function() {
		Pebble.openURL('http://' + SERVER_NAME + '/pebble/' + APP_NAME + '/configure.html');
	}
);


Pebble.addEventListener('webviewclosed', 
	function(e) {
		var options = JSON.parse(decodeURIComponent(e.response));
		var optionKeys = Object.keys(options);
		var key;
		var value;
		var message = {};

		for(var counter = 0; counter < optionKeys.length; counter++) {
			key = optionKeys[counter];
			value = options[key];
			window.localStorage.setItem(key, value);
			message[key] = parseInt(value);	// assumes all values are integers
		}

		Pebble.sendAppMessage(message);
	}
);
