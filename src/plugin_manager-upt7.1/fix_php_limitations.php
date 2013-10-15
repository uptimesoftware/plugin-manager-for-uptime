<?php

// This will update/fix the php.ini to allow larger files than 2MB
// This is executed during the up.time Plugin Manager post-install, but can also be executed after an up.time monitoring station upgrade/update

// import necessary file(s)
include_once('plugin_functions.inc');


// make sure there's only one cmd-line argument (php.ini file path)
if (count($argv) != 2) {
	print("ERROR: Must enter the php.ini file path.\n");
	exit(1);
}
// make sure the file exists
if ( ! file_exists(trim($argv[1])) ) {
	print("ERROR: File '{$argv[1]}' does not exist.\n");
	exit(1);
}


////////////////////////////////////////////////////
// Variables
$php_ini_file = trim($argv[1]);
$new_php_ini_file = $php_ini_file . '.ori';
$new_line = "\n";
if (get_os_platform() == 'win') {	// Windows
	$new_line = "\r\n";
}

// read file
$fh_ori = @fopen($php_ini_file, "r");
$fh_new = @fopen($new_php_ini_file, "w");
// make sure we can write the file
if ( ! $fh_new) {
	print "ERROR: Could not write temp file: '{$new_php_ini_file}'.\n";
	exit(2);
}
if ($fh_ori) {
    while (($line = fgets($fh_ori, 4096)) !== false) {
		$line = trim($line);
		if ( preg_match('/^post_max_size \=/', $line) ) {
			$line  = "post_max_size = 100M";
		}
		elseif ( preg_match('/^upload_max_filesize \=/', $line) ) {
			$line  = "upload_max_filesize = 100M";
		}
		
		// write new file
		fwrite($fh_new, $line . $new_line);
		
    }
    if (!feof($fh_ori)) {
        echo "Error: unexpected fgets() fail" . $new_line;
    }
    fclose($fh_new);
    fclose($fh_ori);
	
	// rename both files
	$tmp_file = $php_ini_file . ".tmp";
	rename($php_ini_file, $tmp_file);			// ori > tmp
	rename($new_php_ini_file, $php_ini_file);	// new > ori
	rename($tmp_file, $new_php_ini_file);		// tmp > new
	
	// delete old file
	unlink($new_php_ini_file);
	
	print "Increased file upload limits so we can import all up.time plugins.\n";
}

exit(0);

?>
