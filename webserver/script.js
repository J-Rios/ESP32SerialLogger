
/* WebSocket Functions */

var websocket = new WebSocket("ws://"+location.hostname+"/");

websocket.onopen = function(evt)
{
	console.log("WebSocket connection opened.");
}

websocket.onclose = function(evt)
{
	console.log("Websocket connection closed.");
}

websocket.onerror = function(evt)
{
	console.log("Websocket error (" + evt.code + "): " + evt.data);
}

websocket.onmessage = function(evt)
{
	//console.log("Received message from websocket: " + evt.data);
	websocket_onmessage(evt);
}

function websocket_send(msg)
{
	websocket.send(msg);
	console.log("Sent message to websocket: " + msg);
}

/**************************************************************************************************/

/* Auxiliar Functions */

function http_post(path, params, method)
{
	method = method || "post";
	var form = document.createElement("form");
	form.setAttribute("method", method);
	form.setAttribute("action", path);
	for(var key in params) {
		if(params.hasOwnProperty(key)) {
			var hiddenField = document.createElement("input");
			hiddenField.setAttribute("type", "hidden");
			hiddenField.setAttribute("name", key);
			hiddenField.setAttribute("value", params[key]);
			form.appendChild(hiddenField);
		}
	}
	document.body.appendChild(form);
	form.submit();
}

function sleep(ms)
{
	return new Promise(resolve => setTimeout(resolve, ms));
}

function blob_to_sring(file)
{
	return new Promise((resolve, reject) => {
		var reader = new FileReader();
		reader.onload = () => {
			resolve(reader.result);
		};
		reader.onerror = reject;
		reader.readAsText(file);
	})
}
