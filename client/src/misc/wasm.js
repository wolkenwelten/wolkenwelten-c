
function wsSendData(data,len) {
	if(!window.wolkenweltenSocketOpen){return 0;}
	/*
	let str = ' ';
	for(let i=0;i<len;i++){
		str += (HEAP8[data+i]+' ');
	}
	console.log('Send['+len+']:' + str);
	*/
	return len;
};

function wsRecvData(buf,size){
	if(!window.wolkenweltenSocketOpen){return 0;}

	return 0;
};

function wsInitClient(server,clientName){
	window.wolkenweltenSocketOpen = false;
	window.wolkenweltenSocket = new WebSocket('ws://'+server+':6309');
	window.wolkenweltenSocket.onopen = function(e){
		console.log("OnOpen "+window.wolkenweltenSocket.readyState);
		window.wolkenweltenSocket.send(clientName+"\n");
		window.wolkenweltenSocketOpen = true;
	}
	window.wolkenweltenSocket.onclose = function(e){
		window.wolkenweltenSocketOpen = false;
	}
	window.wolkenweltenSocket.onerror = function(e){
		window.wolkenweltenSocket.close();
		window.wolkenweltenSocketOpen = false;
	}
	window.wolkenweltenSocket.onmessage = function(e){
		console.log(e.data);
	}
	console.log("Connecting to "+server+' as '+clientName);
};

function wsFreeClient(){
	if(window.wolkenweltenSocketOpen === true){
		window.wolkenweltenSocket.close();
	}
	window.wolkenweltenSocketOpen = false;
	console.log('Closing WebSocket');
};

