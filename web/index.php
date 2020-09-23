<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<?php

$stable       = [];
$experimental = [];
$platforms    = [
	'win'   => 'Windows',
	'macos' => 'MacOS',
	'linux' => 'GNU/Linux'
];

function getReleaseDir(){
	if(is_dir('releases')){return 'releases/';}
	if(is_dir('../releases')){return '../releases/';}
	return false;
}

function checkReleases($platform){
	global $stable,$experimental;

	$arrS = [];
	$arrE = [];
	$dir = getReleaseDir().$platform.'/';
	if(!is_dir($dir)){return;}
	$handle = opendir($dir);
	if ($handle === false){return;}
	while (false !== ($entry = readdir($handle))) {
		if($entry[0] == '.'){continue;}
		if(strpos($entry,"wolkenwelten-$platform") === false){continue;}
		if(strpos($entry,"master") === false){
			$arrS[$entry] = filemtime($dir.$entry);
		}else{
			$arrE[$entry] = filemtime($dir.$entry);
		}
	}
	closedir($handle);
	asort($arrS,SORT_NUMERIC);
	asort($arrE,SORT_NUMERIC);

	$stable[$platform]       = array_reverse($arrS,true);
	$experimental[$platform] = array_reverse($arrE,true);
}

function niceName($platform){
	global $platforms;

	if(isset($platforms[$platform])){
		return $platforms[$platform];
	}
	return $platform;
}

function echoBuilds($arr){
	foreach($arr as $platform => $releases){
		$rel = array_key_first($releases);
		if(trim($rel) == ''){continue;}
		$href = 'release/'.$platform.'/'.$rel;
		$label = '<span class=buttonlabel>'.niceName($platform).'</span><span class="buttonicon icon-'.$platform.'"></span>';
		echo '<a href="'.$href.'" download class=button>'.$label."</a>";
	}
	echo "<br/>\n";
}

foreach(array_keys($platforms) as $plat){
	checkReleases($plat);
}
?>
<html>
<head>
	<meta charset="utf-8"/>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
	<meta name="viewport" content="width=device-width, initial-scale=1"/>
	<title>Wolkenwelten</title>
	<link rel="icon" type="image/png" href="favicon16.png" sizes="16x16">
	<link rel="icon" type="image/png" href="favicon32.png" sizes="32x32">
	<link rel="stylesheet" href="main.css"/>
</head>
<body bgcolor="#333333" text="#cccccc"  link="#cc3366" vlink="#cc3366" alink="#cc3366">
	<h1>Wolkenwelten Alpha</h1>
	<hr/>
	<p>Greetings Cybersurfer, you have reached my little corner of the WWW offering a sneak peek of a videogame I have been developing in my spare time.</p>
	<p>You can watch me work on this Game on my <a href="https://www.twitch.tv/melchizedek6809">Twitch channel</a>, I try to do more visually exciting features on stream but it is still just me coding away, so be prepared for that.</p>
	<p>Apart from that you can also join our <a href="https://discord.gg/vPZffVS">Discord</a>.</p>
	<p>For now only Win/Mac/Lin are supported with a native version, although there is the wasm version which should work on every OS having a modern Browser. Apart from that the codebase is already running and getting regular testing on OpenBSD and Haiku.</p><br/>
	<h2>Download a stable release:</h2>
	<hr/>
	<div class=nativedl align=center>
		<?php echoBuilds($stable); ?>
	</div>
	<br/>
	<br/>
	<h2>Or, try the newest experimental build:</h2>
	<hr/>
	<div class=nativedl align=center>
		<?php echoBuilds($experimental); ?>
		<i>Win32 only, users of other platforms should use wine</i>
	</div>
	<!--
	<hr/>
	<br/>
	<h2>Or play directly in your Browser using WebAssembly</h2>
	<hr/>
	<div class=wasmdl align=center>
		<a href="releases/wasm/index.html" class=button><span class=buttonlabel>Start WASM version</span><span class="buttonicon icon-wasm"></span><br/>
		<b>Only as a client, needs a separate native Server for now</b><br/>
		<i>Tested in latest Chrome/Firefox/Safari</i>
	</div>
	-->
	<hr/>
	<h2>Have some Screenshots</h2>
	<div align=center class=pics>
		<a href="img/1.jpg"><img src="img/1.jpg" width=1920 height=1080/></a>
		<a href="img/2.jpg"><img src="img/2.jpg" width=1920 height=1080/></a>
		<a href="img/3.jpg"><img src="img/3.jpg" width=1920 height=1080/></a>
	</div>
	<hr/>
	<h3>My Priorities for this Game</h3>
	<h4>Cooperation over Competition</h4>
	<p>Playing together as a group in a survival sandbox and fighting a common enemy have been some of the most enjoyable experiences I had in gaming, that is why I want to put most of my time into optimizing for this style instead of players competing with each other (apart from that there are already a lot of competitive shooters out there).</p>
	<h4>Exploration over Grinding</h4>
	<p>I try to keep the grinding to a minimum, which is one of the reasons why I do not want players to have to mine for resources, instead players should explore and/or fight to progress.</p>
	<h4>Gameplay over Beauty</h4>
	<p>Since computing power is set by the players hardware I can only decide where to spend those cycles on, which will most likely be something enhancing gameplay.</p>
	<h4>Experimentation over Planing</h4>
	<p>I greatly prefer just experimenting around with features and see what is working and what is not, even if this means throwing out old working features.</p>
	<h4>World-centric instead of Player-centric</h4>
	<p>Most games focus solely on the players actions where pretty much everything needs to be initiated by the player, here I want to try out to put the world in the center, where things might just happen without anybody noticing or having something to do with it. One thing I would love to reach would be for it to be an enjoyable experience to just sit in-game and watch the scenery and events unfold.</p>
	<h4>Performance over Bloat</h4>
	<p>Performance is quite important to me because there are a lot of people who only have cheap laptops/phones instead of big gaming desktops. This combined with the focus on cooperative multiplayer forces me to pay extra attention to Performance/Bloat.</p>
	<hr/>
	<h2>Controls</h2>
	<h4>Mouse / Keyboard</h4>
	<p><span class=key>W</span>,<span class=key>S</span>,<span class=key>A</span>,<span class=key>D</span> to move around the world.</p>
	<p><span class=key>Spacebar</span> to jump.</p>
	<p><span class=key>Shift</span> to sneak.</p>
	<p><span class=key>E</span> to shoot/retrieve your grappling hook.</p>
	<p><span class=key>Tab</span> open/close inventory/crafting popup.</p>
	<p><span class=key>LMB</span> to destroy a block.</p>
	<p><span class=key>Mousewheel</span> or <span class=key>1...0</span> to select an item..</p>
	<p><span class=key>RMB</span> to use the current item.</p>
	<p><span class=key>V</span> for a glider.</p>
	<p><span class=key>Q</span> to drop the current item.</p>
	<p><span class=key>N</span> to start flying around in no-clip mode.</p>
	<br/>
	<h4>Gamepad</h4>
	<img src="img/padcontrols.png" width=900 height=504 class="imgfw" />
	<p><i>The layout is based on an Xbox360 controller, if you have a different controller it might differ but should be in a similar physical location.</i></p>
	<p>Use the left analog stick to walk around and the right analog stick to look around. Use the D-Pad to select the active item. A for jumping and Y for dropping items. Use the left shoulder trigger for activating the current item and the right one for destroying blocks, the right shoulder button shoots the grappling hook. To open the Inventory press the left Menu button, most probably labeled "Back".</p>
	<br/>
	<h4>Touchscreen</h4>
	<p><b>These controls are really nasty and you should try to get another input device somehow!</b></p>
	<p>Put your thumb on the lower left corner of the screen to move around, in the lower right corner is a zone for looking around. The upper area of the screen is separated into 4 zones used for dropping items, jumping, activating items and destroying blocks. You might wonder how you might select an item, you can't right now ;) Improved touchscreen controls will have to wait until a later time, the current controls were mostly done as a proof of concept.</p>
	<hr/>
</body>
