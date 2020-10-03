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

function echoBuilds($arr){
	foreach($arr as $platform => $releases){
		$rel = array_key_first($releases);
		if(trim($rel) == ''){continue;}
		$href = 'release/'.$platform.'/'.$rel;
		$label = '<span class=buttonlabel>'.niceName($platform).'</span><span class="buttonicon icon-'.$platform.'"></span>';
		echo '<a href="'.$href.'" download class=button title="'.niceDate($platform.'/'.$rel).'">'.$label."</a>";
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