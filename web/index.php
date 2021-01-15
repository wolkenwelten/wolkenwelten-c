<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<?php

require "func.php";

$stable       = [];
$experimental = [];
$platforms    = [
	'win'   => 'Windows',
	'macos' => 'MacOS',
	'linux-x86_64' => 'GNU/Linux',
	'linux-aarch64' => 'Linux ARM 64-Bit'
];

foreach(array_keys($platforms) as $plat){
	checkReleases($plat);
}

?>
<html>
<head>
	<meta charset="utf-8"/>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
	<meta name="viewport" content="width=device-width, initial-scale=1"/>
	<title>WolkenWelten</title>
	<link rel="icon" type="image/png" href="<?php echo baseSF('favicon16.png'); ?>" sizes="16x16">
	<link rel="icon" type="image/png" href="<?php echo baseSF('favicon32.png'); ?>" sizes="32x32">
	<style>
	<?php echo file_get_contents("main.css"); ?>
	</style>
</head>
<body bgcolor="#333333" text="#cccccc"  link="#cc3366" vlink="#cc3366" alink="#cc3366">
	<h1>WolkenWelten - Alpha</h1>
	<hr/>
	<p>Greetings <b><?php echo hyper("Cybersurfer"); ?></b>, you have reached my little corner of the WWW offering a sneak peek of a videogame I have been developing in my spare time.</p>
	<p>You can watch me work on this Game on my <a href="https://www.twitch.tv/melchizedek6809">Twitch channel</a>, I try to do more visually exciting features on stream but it is still just me coding away, so be prepared for that.</p>
	<p>To find other People to play with be sure to Join our <a href="https://discord.gg/vPZffVS">Discord</a>.</p>
	<p>For now only Win/Mac/Lin are supported with a native Release. WASM is working, but for now still has too many Bugs. Haiku/OpenBSD are working as well but I still haven't gotten around to building a proper release pipeline for them.</p><br/>
	<h2>Download a stable release:</h2>
	<hr/>
	<div class=nativedl align=center>
		<?php echoBuilds($stable); ?>
		<i>Current versions are not even close to stable, but you may try an experimental build below.</i>
	</div>
	<br/>
	<br/>
	<h2>Or, try the latest and greatest experimental build:</h2>
	<hr/>
	<div class=nativedl align=center>
		<?php echoBuilds($experimental); ?>
	</div>

	<hr/>
	<br/>
	<!--
	<h2>Or play directly in your Browser using WebAssembly</h2>
	<hr/>
	<div class=wasmdl align=center>
		<a href="releases/wasm/index.html?worldSeed=2600&savegame=Test" class=button><span class=buttonlabel>Start WASM version</span><span class="buttonicon icon-wasm"></span></a><br/>
		<b>WARNING: Still really unstable, will most likely crash after a minute or two.</b><br/>
		<i>Tested in latest Chrome/Firefox/Safari</i>
	</div>
	<hr/>
	<br/>
	-->

	<h2>Have some Screenshots</h2>
	<div align=center class=pics>
		<a href="img/1.jpg"><img src="<?php echo baseSFS('img/1.jpg'); ?>" width=1920 height=1080/></a>
		<a href="img/2.jpg"><img src="<?php echo baseSFS('img/2.jpg'); ?>" width=1920 height=1080/></a>
		<a href="img/3.jpg"><img src="<?php echo baseSFS('img/3.jpg'); ?>" width=1920 height=1080/></a>
	</div>
	<hr/>
	<h3>My priorities for this game</h3>
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
	<p><span class=key>Spacebar</span> to jump, open you glider if mid-air, or pull in your grappling hook.</p>
	<p><span class=key>Shift</span> to Boost.</p>
	<p><span class=key>E</span> to shoot/retrieve your grappling hook.</p>
	<p><span class=key>Tab</span> open/close inventory/crafting popup.</p>
	<p><span class=key>LMB</span> to destroy a block.</p>
	<p><span class=key>Mousewheel</span> or <span class=key>1...0</span> to select an item..</p>
	<p><span class=key>RMB</span> to use the current item.</p>
	<p><span class=key>R</span> to reload.</p>
	<p><span class=key>V</span> to loosen your grappling hook.</p>
	<p><span class=key>Q</span> to drop the current item.</p>
	<br/>
	<p><span class=key>N</span> to start flying around in no-clip mode.</p>
	<p><span class=key>M</span> to toggle the debug overlayo.</p>
	<p><span class=key>F2</span> to open/close your inventorye.</p>
	<p><span class=key>F3</span> to open/close the crafting panel.</p>
	<p><span class=key>F11</span> to toggle Fullscreen mode.</p>
	<br/>
	<h4>Gamepad</h4>
	<img src="<?php echo baseSF('img/padcontrols.png'); ?>" width=900 height=504 class="imgfw" />
	<p><i>The layout is based on an Xbox360 controller, if you have a different controller it might differ but should be in a similar physical location.</i></p>
	<p>Use the left analog stick to walk around and the right analog stick to look around. Use the D-Pad to select the active item. A for jumping and Y for dropping items. Use the left shoulder trigger for activating the current item and the right one for destroying blocks, the right shoulder button shoots the grappling hook. To open the Inventory press the left Menu button, most probably labeled "Back".</p>
	<br/>
	<h4>Touchscreen</h4>
	<p><b>These controls are really nasty and you should try to get another input device somehow!</b></p>
	<p>Put your thumb on the lower left corner of the screen to move around, in the lower right corner is a zone for looking around. The upper area of the screen is separated into 4 zones used for dropping items, jumping, activating items and destroying blocks. You might wonder how you might select an item, you can't right now ;) Improved touchscreen controls will have to wait until a later time, the current controls were mostly done as a proof of concept.</p>
	<hr/>
	<center><h4>Imprint</h4></center>
	<p><center>Benjamin Vincent Schulenburg - Steinmetzstr. 24 - 45139 Essen - GERMANY</center></p>
	<center><h4>Contact</h4></center>
	<p><center>Discord gets checked most often, but you can also send me an <a href="mailto:ben@wolkenwelten.net">E-Mail</a>, although it might take a while for me to get to it.</center></p>
</body>
