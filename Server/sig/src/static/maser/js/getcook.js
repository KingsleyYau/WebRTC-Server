function get_cookie(varname){
	var tmp_ary = new Array();
	if (varname){
		a = document.cookie.indexOf(varname+"=");
		if (a != -1){
			b = document.cookie.substring((a+varname.length+1),document.cookie.length);
			c = b.split(";");
			d = c[0];
			return d;
		}
	}
}
function setCookie(name, value){
	var argv = setCookie.arguments;
	var argc = setCookie.arguments.length;
	var expDay = (argc > 2) ? argv[2] : 30;
	try{
		expDay=parseFloat(expDay);
		if(expDay<0)expDay=0;
	}catch(e){
		expDay=30;
	}
	var expDate = new Date();
	// The expDate is the date when the cookie should expire, we will keep it for a month
	expDate.setTime( expDate.getTime() + (expDay * 24 * 60 * 60 * 1000) ); 
	setCookieVal( name, value, expDate,'/','.zol.com.cn'); 	
}
function setCookieVal(name, value){
	var argv = setCookieVal.arguments;
	var argc = setCookieVal.arguments.length;
	var expires = (argc > 2) ? argv[2] : null;
	var path = (argc > 3) ? argv[3] : null;
	var domain = (argc > 4) ? argv[4] : null;
	var secure = (argc > 5) ? argv[5] : false;
	document.cookie = name + "=" + escape (value) +
	((expires == null) ? "" : ("; expires=" + expires.toGMTString())) +
	((path == null) ? "" : ("; path=" + path)) +
	((domain == null) ? "" : ("; domain=" + domain)) +
	((secure == true) ? "; secure" : "");
}
function deleteCookie(name){
	var exp = new Date();
	exp.setTime (exp.getTime() - 1); // This cookie is history
	var cval = get_cookie (name);
	document.cookie = name + "=" + cval + "; expires=" + exp.toGMTString();
}
function genFlash(flash_src,flash_id,div_str,width,height,trans_tag){
	var return_str = "";
	return_str += '<object id="'+flash_id+'" classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000" codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,29,0" width="'+width+'" height="'+height+'"><param name="SRC" value="'+flash_src+'" />';
	if(1==trans_tag)
		return_str += '<param name="wmode" value="transparent" />';
	return_str += '<embed id="'+flash_id+'" src="'+flash_src+'" quality="high" pluginspage="http://www.macromedia.com/go/getflashplayer" type="application/x-shockwave-flash" width="'+width+'" height="'+height+'"';
	if(1==trans_tag)
		return_str += ' wmode="transparent"';
	return_str += '></embed></object>';
	if(""!=div_str){
		document.getElementById(div_str).innerHTML = return_str;
	}else{
		return return_str;
	}
}

var tmp_name = document.cookie.indexOf('Adshow=');
var Adshow = get_cookie('Adshow');
if(-1==tmp_name){
	setCookie('Adshow',1);
}else{
	Adshow = Adshow - 1;
	if(-1==Adshow) Adshow=5;
	setCookie('Adshow',Adshow);
}
//alert(Adshow);

var userid = get_cookie("zol_userid");
var nickname = get_cookie("zol_nickname");
var names =nickname ? nickname : userid;
function filterStrChar(str) {
	if (str == undefined) { return ''; }
	str = str.replace(/<\/?[^>]*>/g,'').replace(/[ | ]*\n/g,'\n').replace(/\n[\s| | ]*\r/g,'\n').replace(/['"]*/g,'').replace(/=*/g,'').replace(/>*/g,'').replace(/<*/g,'');return str;}
userid = filterStrChar(userid);
names = filterStrChar(names); 
var backUrl = document.URL;