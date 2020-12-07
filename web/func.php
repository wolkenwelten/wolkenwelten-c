<?php

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

function niceDate($name){
	$path = getReleaseDir().$name;
	$mtime = filemtime($path);
	return "Released: ".date("Y-m-d H:i:s",$mtime);
}

function humanFileSize($size) {
	if($size >= 1<<20){
		return number_format($size/(1<<20),1)." MB";
	}
	if($size >= 1<<10){
		return number_format($size/(1<<10),1)." KB";
	}
	return number_format($size)." B";
}

function niceSize($name){
	$path  = getReleaseDir().$name;
	$fsize = filesize($path);
	return "Size: ".humanFileSize($fsize);
}

function echoBuilds($arr){
	foreach($arr as $platform => $releases){
		$rel = array_key_first($releases);
		if(trim($rel) == ''){continue;}
		$href = 'releases/'.$platform.'/'.$rel;
		$label = '<span class=buttonlabel>'.niceName($platform).'</span><span class="buttonicon icon-'.$platform.'"></span>';
		echo '<a href="'.$href.'" download class=button title="'.niceDate($platform.'/'.$rel).' - '.niceSize($platform.'/'.$rel).'">'.$label."</a>";
	}
	echo "<br/>\n";
}

function baseSF($file){
	$ext = strtolower(pathinfo($file, PATHINFO_EXTENSION));
	switch($ext){
		case 'jpeg':
		case 'jpg':
		default:
			$prefix = 'data:image/jpg;base64,';
			break;
		case 'png':
			$prefix = 'data:image/png;base64,';
			break;
		case 'gif':
			$prefix = 'data:image/gif;base64,';
			break;
	}

	return $prefix.base64_encode(file_get_contents($file));
}

function baseSFS($file){
	$tmpf = tempnam("/tmp", "wolkenwelten").'.jpg';
	system('gm convert -size 281x158 "'.$file.'" -resize 281x158 +profile "*" -quality 85 "'.$tmpf.'"');
	$ret = baseSF($tmpf);
	unlink($tmpf);
	return $ret;
}

function interpolate(string $c1,string $c2,float $v){
	$r1 = (hexdec($c1) & 0xFF0000) >> 16;
	$g1 = (hexdec($c1) & 0x00FF00) >>  8;
	$b1 = (hexdec($c1) & 0x0000FF)      ;

	$r2 = (hexdec($c2) & 0xFF0000) >> 16;
	$g2 = (hexdec($c2) & 0x00FF00) >>  8;
	$b2 = (hexdec($c2) & 0x0000FF)      ;

	$r  = (($r1 * (1.0 - $v)) + ($r2 * $v));
	$g  = (($g1 * (1.0 - $v)) + ($g2 * $v));
	$b  = (($b1 * (1.0 - $v)) + ($b2 * $v));

	return sprintf("%02x%02x%02x", $r, $g, $b);
}

function hyper($str,$from='ff4488',$to='44ff66'){
	$new = '';
	$chars = str_split($str);
	$len = count($chars);
	$i = 0;
	$c1 = $from;
	$c2 = $to;
	foreach($chars as $char){
		$v = $i++ / ($len-1);
		$c = interpolate($c1,$c2,$v);

		$new .= '<span style="color:#'.$c.'">'.htmlspecialchars($char).'</span>';
	}
	return $new;
}
