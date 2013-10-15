<?php
////////////////////////////////////////////////////////
// Variables
$this_dir = getcwd();
chdir("../..");
$uptime_dir = getcwd() . "/";	// uptime directory; should be something like "(uptime_dir)/GUI/plugin_importer"
chdir($this_dir);
$main_dir = fix_directory_slashes($uptime_dir . "{$plugins_dir_name}/");	// plugins main directory; should be something like "(uptime_dir)/GUI/plugin_manager"
$tmp_dir = fix_directory_slashes($main_dir . "temp/tmp_" . getmypid() . "/");
$downloads_dir = fix_directory_slashes($main_dir . "downloads/");
// create the download directory if it doesn't already exist
if (! file_exists($downloads_dir)) {
	mkdir($downloads_dir, 0777, true) or die("Error: Could not create downloads directory: '{$downloads_dir}'\n");
}
if (! file_exists($tmp_dir)) {
	mkdir($tmp_dir, 0777, true) or die("Error: Could not create temp directory: '{$tmp_dir}'\n");
}

?>