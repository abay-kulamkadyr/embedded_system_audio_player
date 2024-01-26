"use strict";
// Client-side interactions with the browser.

// Make connection to server when web page is fully loaded.

var socket = io.connect();
$(document).ready(function() {
	
	$('#skipforw').click(function(){
		sendPrimeCommand("SH\n");
	});
	$('#skipback').click(function(){
		sendPrimeCommand("SB\n");
	});
	$('#pause').click(function(){
		sendPrimeCommand("P\n");
	});
	$('#moveback').mousedown(function(){
		sendPrimeCommand("RT\n");
		
	});
	$('#moveback').mouseup(function(){
		sendPrimeCommand("RT\n");
	});
	$('#moveforw').mousedown(function(){
		sendPrimeCommand("FT\n");
	});
	$('#moveforw').mouseup(function(){
		sendPrimeCommand("FT\n");
	});
	//$('#volumeUp').click(function(){
	// 	volumeDisplayUpdate();
	// 	sendPrimeCommand("VU\n");
	// });
	// $('#volumeDown').click(function(){
	// 	volumeDisplayUpdate();
	// 	sendPrimeCommand("VD\n");
	// });
	
	
	socket.on('commandReply', function(result) {
		if (result!="Done\n"){
			$('#displaytrack').text(result);
		}
		//var newDiv = $('<div></div>').text(result);
		/*if (result.charAt(0)=='N' || result.charAt(0)=='R'){
			var ctext=String("Current drum beat mode is: ").concat(result);
			$('#displaysound').text(ctext);
		}
		else if (result.charAt(0)=='V'){
			
			$('#displayvol').text(result.split(" ")[1]);
		}
		
		else if (result.charAt(0)=='P'){
			var tottime=parseInt(result.split(" ")[1]);
			var hours=Math.floor(tottime/3600);
			tottime=tottime-(hours*3600);
			var minutes=Math.floor(tottime/60);
			tottime=tottime-(minutes*60);
			var seconds=Math.floor(tottime);
			if (seconds<10){
				seconds="0".concat(String(seconds));
			}
			var str=String(hours).concat(":",String(minutes),":",String(seconds)," ","(H:M:S)");
			$('#uptime').text(str);
		}
		else{
			$('#displaytempo').text(result.split(" ")[1]);
		}*/
	});
	//socket.on('volumeReceived', function(result)
	// {
	// 	console.log("result= ", result)
	// 	$('#volumeDisplay').val(result)
	// });
	
});


$(document).ready(function() {
	window.setInterval(function() {updateInfo()},600);
});



function updateInfo(){
	sendPrimeCommand("Ctrack\n");
}


function sendPrimeCommand(message) {
	socket.emit('prime', message);

};





// function volumeDisplayUpdate(buttonNum){
// 	if(buttonNum == 1){
// 		var curr_vol =  document.getElementById("volumeDisplay").value;
// 		curr_vol++;
// 		document.getElementById("volumeDisplay").value = curr_vol;
// 	}
// 	else if (buttonNum == 2){
// 		var curr_vol =  document.getElementById("volumeDisplay").value;
// 		curr_vol--;
// 		document.getElementById("volumeDisplay").value = curr_vol;

// 	}
// };