
function wsSendData(data,len) {
	if(!window.wolkenweltenSocketOpen){return 0;}
	let view = new Uint8Array(wasmMemory.buffer, data, len);
	window.wolkenweltenSocket.send(view);
	return len;
};

function wsRecvData(buf,size){
	if(!window.wolkenweltenSocketOpen)     {return 0;}
	if(window.wolkenweltenRecvBufLen == 0) {return 0;}

	if(size > window.wolkenweltenRecvBufLen){
		size = window.wolkenweltenRecvBufLen;
	}
	let src = new Uint8Array(window.wolkenweltenRecvBuf,0,size);
	let dst = new Uint8Array(wasmMemory.buffer,buf,size);
	dst.set(src);
	window.wolkenweltenRecvBufLen -= size;

	return size;
};

function wsRecvAB(data){
	if(window.wolkenweltenRecvBuf === undefined){return;}
	if(window.wolkenweltenRecvBufLen + data.byteLength > window.wolkenweltenRecvBufSize){
		console.log("RecvBuf overflow!!!");
		return;
	}
	let src = new Uint8Array(data,0,data.byteLength);
	let dst = new Uint8Array(window.wolkenweltenRecvBuf,window.wolkenweltenRecvBufLen,src.byteLength);
	dst.set(src);
	window.wolkenweltenRecvBufLen += src.byteLength;
}

function wsRecvHandler(e){
	if(e.data instanceof Blob){
		new Response(e.data).arrayBuffer().then(
			(data) => { wsRecvAB(data); },
			(err)  => { console.log(err); }
		);
	}else if(e.data instanceof ArrayBuffer){
		wsRecvAB(e.data)
	}else{
		console.log(e.data);
	}
}

function wsInitClient(server,clientName){
	window.wolkenweltenSocketOpen = false;

	window.wolkenweltenRecvBufSize = 1024*1024*4;
	window.wolkenweltenRecvBuf     = new ArrayBuffer(window.wolkenweltenRecvBufSize);
	window.wolkenweltenRecvBufLen  = 0;

	window.wolkenweltenSocket = new WebSocket('ws://'+server+':6309',["binary"]);
	window.wolkenweltenSocket.onopen = function(e){
		window.wolkenweltenSocketOpen = true;
	}
	window.wolkenweltenSocket.onclose = function(e){
		window.wolkenweltenSocketOpen = false;
	}
	window.wolkenweltenSocket.onerror = function(e){
		console.log(e);
		window.wolkenweltenSocketOpen = false;
	}
	window.wolkenweltenSocket.onmessage = wsRecvHandler;
	console.log("Connecting to "+server+' as '+clientName);
};

function wsFreeClient(){
	if(window.wolkenweltenSocketOpen === true){
		window.wolkenweltenSocket.close();
	}
	window.wolkenweltenRecvBufView = undefined;
	window.wolkenweltenRecvBuf     = undefined;
	window.wolkenweltenSocketOpen  = false;
	console.log('Closing WebSocket');
};
