samplelist = {"ses":"SESNOTSPECIFIED",
		"camEnabled":False

}

page = """


<head> 
<script> 
 
 
function init()
{
	counter = 0;
	camT = 200;
	comT = 100;
	baseURL = "/wsgi/tcp/"; 
	imgObj = document.getElementById("mainImage");
	parentObj = document.getElementById("imgHolster");
 
	newImg = document.createElement('img');
 
	newImg.setAttribute('src', baseURL+'fetchImage');
	newImg.setAttribute('width', '604');
	newImg.setAttribute('height', '450');
	//alert(newImg.complete.toString());
	#if $camEnabled
		waitForLoaded();
	#end if
	//newImg.onLoad = alert("new loaded");
 
	//imgObj.onLoad = alert("loaded");
	//imgObj.src = "http://ss/wsgi/template/";
 
	camX = 0;
	camY = 0;
	carX = 0;
	carY = 0;
 
	carT = 150;
	carL = 150;
	camT = 150;
	camL = 150;

	ses = "$ses"; 
 
 
	mouseDown = false;
	leftDown = false;
	rightDown = false;
	upDown = false;
	downDown = false;
	
	 
	xrq = new XMLHttpRequest();
	xrq.open("GET", genURL(), true);
	xrq.send(null);
	cmdLoop();
	
}

function genURL()
{
	/*
	This generates the GET url for the command XML script
	*/

	var result = baseURL+"command?ses="+ses+"&v="+camY.toString()+"&h="+camX.toString()+"&d="+carX.toString()+"&s="+carY.toString();
	return result;
}
 
function waitForLoaded()
{
	if(!(newImg.complete))
	{
		//alert("notLoaded");
		setTimeout("waitForLoaded()", camT);
		return;
	}
	//alert("LOADED");
	swapImages();
	fetchNextImage();
	waitForLoaded();
}
 
function swapImages()
{
	imgObj = parentObj.childNodes[0];
	parentObj.removeChild(imgObj);
	parentObj.appendChild(newImg);
}
 
function fetchNextImage()
{
	sURL = baseURL+"fetchImage?i="+counter.toString();
	counter++;
	newImg = document.createElement('img');
	newImg.setAttribute('src', sURL);
	newImg.setAttribute('width', '604');
	newImg.setAttribute('height', '450');
}
 
function isLoaded()
{
	alert(newImg.complete.toString());
}
 
function moveStickCenter(par, chi, x, y)
{
	/*
	This will move the child to in the parent so the center is
	on x and y.
	*/
	/*
	var pWidth = parseInt(par.style.width);
	var pHeight = par.style.height;
	var cWidth = chi.style.width + chi.style.borderWidth;
	var cHeight = chi.style.height + chi.style.borderWidth;
	*/
	var pWidth = 300;
	var pHeight = 300;
	var cWidth = 50;
	var cHeight = 50;
	
	var hBorder = cWidth/2;
	var vBorder = cHeight/2;
 
	var posx = 0;
	var posy = 0;
 
	if(x < hBorder)
		posx = 0;
	else if (x > (pWidth-hBorder))
		posx = pWidth-cWidth;
	else
		posx = x-hBorder;
 
	
	if(y < vBorder)
		posy = 0;
	else if (y > (pHeight-vBorder))
		posy = pHeight-cHeight;
	else
		posy = y-vBorder;
 
	chi.style.marginLeft = posx+"px";
	chi.style.marginTop = posy+"px";
 
	//Save these values for later (difficult to read poking into the style object)
	
	var prefix = chi.id.substr(0,3);
 
	if(prefix == "cam")
	{
		camT = posy;
		camL = posx;
	} else if (prefix == "car")
	{
		carT = posy;
		carL = posx;
	}
	
}	
 
 
/**
 * Retrieve the absolute coordinates of an element.
 *
 * @param element
 *   A DOM element.
 * @return
 *   A hash containing keys 'x' and 'y'.
 */
function getAbsolutePosition(element) {
  var r = { x: element.offsetLeft, y: element.offsetTop };
  if (element.offsetParent) {
    var tmp = getAbsolutePosition(element.offsetParent);
    r.x += tmp.x;
    r.y += tmp.y;
  }
  return r;
};
 
 
/**
   * Retrieve the coordinates of the given event relative to the center
   * of the widget.
   *
   * @param event
   *   A mouse-related DOM event.
   * @param reference
   *   A DOM element whose position we want to transform the mouse coordinates to.
   * @return
   *    A hash containing keys 'x' and 'y'.
   */
 
//This function was supplied via http://acko.net/blog/mouse-handling-and-absolute-positions-in-javascript
  function getRelativeCoordinates(event, reference) {
    var x, y;
    event = event || window.event;
    var el = event.target || event.srcElement;
 
    if (!window.opera && typeof event.offsetX != 'undefined') {
      // Use offset coordinates and find common offsetParent
      var pos = { x: event.offsetX, y: event.offsetY };
 
      // Send the coordinates upwards through the offsetParent chain.
      var e = el;
      while (e) {
        e.mouseX = pos.x;
        e.mouseY = pos.y;
        pos.x += e.offsetLeft;
        pos.y += e.offsetTop;
        e = e.offsetParent;
      }
 
      // Look for the coordinates starting from the reference element.
      var e = reference;
      var offset = { x: 0, y: 0 }
      while (e) {
        if (typeof e.mouseX != 'undefined') {
          x = e.mouseX - offset.x;
          y = e.mouseY - offset.y;
          break;
        }
        offset.x += e.offsetLeft;
        offset.y += e.offsetTop;
        e = e.offsetParent;
      }
 
      // Reset stored coordinates
      e = el;
      while (e) {
        e.mouseX = undefined;
        e.mouseY = undefined;
        e = e.offsetParent;
      }
    }
    else {
      // Use absolute coordinates
      var pos = getAbsolutePosition(reference);
      x = event.pageX  - pos.x;
      y = event.pageY - pos.y;
    }
    // Subtract distance to middle
    return { x: x, y: y };
  }
 
function testy(evt)
{
 
	
    	var el = evt.target || evt.srcElement;
	var prefix = el.id.substr(0,3);
	
	var ele = document.getElementById(prefix+"Control");
	var stick = document.getElementById(prefix+"Stick");
	var val = getRelativeCoordinates(evt, ele);
	var tt = document.getElementById("tText");
	tt.innerHTML = "X:"+val.x.toString()+" Y:"+val.y.toString()+" "+el.id;
 
	moveStickCenter(ele, stick, val.x, val.y);
}
 
function onMove(evt)
{
	if(!mouseDown)
		return;
 
	
    	var el = evt.target || evt.srcElement;
	var prefix = el.id.substr(0,3);
	var suffix = el.id.substr(3,el.id.length);
	var ele = document.getElementById(prefix+"Control");
	var stick = document.getElementById(prefix+"Stick");
	var val = getRelativeCoordinates(evt, ele);
 
	
	moveStickCenter(ele, stick, val.x, val.y);
}
 
function onUnclick(evt)
{
	mouseDown = false;
	
    	var el = evt.target || evt.srcElement;
	var prefix = el.id.substr(0,3);
	var suffix = el.id.substr(3,el.id.length);
	var ele = document.getElementById(prefix+"Control");
	var stick = document.getElementById(prefix+"Stick");
	var val = getRelativeCoordinates(evt, ele);
 
	
	moveStickCenter(ele, stick, 150, 150);
}
 
function onClick(evt)
{
 
    	var el = evt.target || evt.srcElement;
	var prefix = el.id.substr(0,3);
	var suffix = el.id.substr(3,el.id.length);
	var ele = document.getElementById(prefix+"Control");
	var stick = document.getElementById(prefix+"Stick");
	var val = getRelativeCoordinates(evt, ele);
	
	
	//if(suffix == "Stick") //on the stick!
	//	mouseDown = true;
	if(val.x > 150-25 && val.x < 150+25 && val.y > 150-25 && val.y < 150+25)
		mouseDown = true;
	else
		return;
	
	moveStickCenter(ele, stick, val.x, val.y);
	
}
 
//http://unixpapa.com/js/key.html
function keyDown(event)
{
	var a = "a";
	var b = "b";
	if (event.which == null)
		a = event.keyCode;    // IE
	else if (event.which != 0 && event.charCode != 0)
		a = event.which;	  // All others
  	else
     		a = event.which;
 
	if(a == 38) //up
		upDown = true;
	if(a == 40) //DOWN
		downDown = true;
	if(a == 37) //LEFT
		leftDown = true;
	if(a == 39)
		rightDown = true;
 
	var tt = document.getElementById("tText");
	tt.innerHTML = "PRESSED: "+b;
	repositionCarStick();
	
}
 
function keyUp(event)
{
	var a = "a";
	var b = "b";
	if (event.which == null)
		a = event.keyCode;    // IE
	else if (event.which != 0 && event.charCode != 0)
		a = event.which;	  // All others
  	else
     		a = event.which;
 
	if(a == 38) //up
		upDown = false;
	if(a == 40) //DOWN
		downDown = false;
	if(a == 37) //LEFT
		leftDown = false;
	if(a == 39)
		rightDown = false;
 
	var tt = document.getElementById("tText");
	tt.innerHTML = "RELEASED: "+b;
	repositionCarStick();
	
}
 
function repositionCarStick()
{	
	/*
	This will position the car stick according to keys pressed
	*/
 
	var chi = document.getElementById("carStick");
	var par = document.getElementById("carControl");
	var x = 150;
	var y = 150;
	var move = 100;
	if(downDown)
		y += move;
	else if (upDown)
		y -= move;
	if(rightDown)
		x += move;
	else if (leftDown)
		x -= move;
	
	moveStickCenter(par, chi, x, y);
	generateValues();
	
}
 
function convertScale(oCenter, oWidth, tWidth, tCenter, val)
{
	/*
	will scale origin to trage scale and centers
	*/
	val -= oCenter;
	var rat = tWidth/oWidth;
	
	val *= rat;
 
	val += tCenter;
 
	return val;
	
	
 
}
 
function generateValues()
{
	/*
	this will read the position of the cam and car stick
	and convert this to a valu of +100 <-> -100
	*/
	
 
	var tt = document.getElementById("cmdText");	
	camX = Math.round(convertScale(125, 250, 200, 0, camL));
	camY = -1*Math.round(convertScale(125, 250, 200, 0, camT));
	carX = Math.round(convertScale(125, 250, 200, 0, carL));
	carY = -1*Math.round(convertScale(125, 250, 200, 0, carT));
 
	tt.innerHTML = "cX: "+camX.toString()+" cY: "+camY.toString()+" dX: "+carX.toString()+ " dY: "+carY.toString();
	
 
	
}
 
function yesno(val)
{
	if (val == "YES")
		return true;
	return false;
}


 
function checkXML()
{
	
	//document.write(xrq.responseText);
	//Getting the body?

	var xml = xrq.responseXML;
	var data = xml.childNodes[0];
	var sControl = data.childNodes[0].textContent;
	var sValid = data.childNodes[1].textContent;



	control = yesno(sControl);
	valid = yesno(sValid);
 
}


function cmdLoop()
{
	
	if(xrq.readyState != 4)
	{
		setTimeout("cmdLoop()", comT);
			return;
	}
	
	//Okay, verify the response
	checkXML();

	if(!valid && !control)
	{
		window.location = "./error?err=INVALIDSES"
	}	
	else if (!valid && control)
	{
		var tt = document.getElementById("cmdText");
		tt.innerHTML = "COMMS ERROR";
	}

	generateValues();
	
 
	
 	//Generate new request!
	xrq = new XMLHttpRequest();
	xrq.open("GET", genURL(), true);
	xrq.send(null);
	
	
	setTimeout("cmdLoop()", 20);
	
 
}
 
</script> 
 
<style type="text/css"> 
#camControl
{
	width: 300px;
	height: 300px;
	border-width: 1px;
	border-style: solid;
	border-color: #000000;
	background: #0000FF;
	float: left;
}
 
#camStick
{
	width:48px;
	height:48px;
	border-width:1px;
	border-style: solid;
	border-color: #FFFFFF;
	background: #444444;
	margin-left:125px;
	margin-top:125px;
}
 
#carControl
{
	width: 300px;
	height: 300px;
	border-width: 1px;
	border-style: solid;
	border-color: #000000;
	background: #00FF00;
	z-index: 1;
}
 
#carStick
{
	width:48px;
	height:48px;
	border-width:1px;
	border-style: solid;
	border-color: #FFFFFF;
	background: #444444;
	margin-left:125px;
	margin-top:125px;
	z-index: 3;
}
 
#carBlock
{
	width: 300px;
	height:300px;
	/*border-width:1px;
	border-style: solid;
	background: #FF0000;*/
	position: absolute;
	z-index: 5;
 
}
 
</style> 
 
 
 
</head> 
<body onLoad="init()" onKeyDown="keyDown(event);" onkeyUp="keyUp(event);"> 
<div id="imgHolster"><img id="mainImage"></div> 
 
<div style="float:left;"> 
	<div id="carBlock" onmousedown="onClick(event);" onmousemove="onMove(event);" onmouseup="onUnclick(event);" onmouseout="onUnclick(event);"></div> 
	<div id="carControl" onmousedown="onClick(event);"> 
		<div id="carStick"></div> 
	</div> 
</div> 
 
<div id="camControl" onmousedown="testy(event);"> 
	<div id="camStick"></div> 
</div> 
<div style="clear:both;"> 
 
 
<div>another<span id="tText">AAA</span></div> 
<div><span id="cmdText">CMD Sending:</span></div> 
</body> 

"""
