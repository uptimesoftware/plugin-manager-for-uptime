<?php
$plugins_dir_name = 'plugin_manager';

require_once("header.php");
include_once("../../{$plugins_dir_name}/pdb_functions.inc");
include_once("../../{$plugins_dir_name}/plugin_classes.inc");
require_once("../../{$plugins_dir_name}/plugin_functions.inc");
require_once("setup_directories.inc");	// save for last

if (isset($_REQUEST["id"])) {
	$id = intval(trim($_REQUEST["id"]));
	// go back if a number was not entered
	if ( ! is_int($id)) {
		header("location: index.php");
	}
}
else {
	// go back to original page
	header("location: index.php");
}

// get plugin from the ID received
$loaded_plugins = new InstalledPlugins();
$loaded_plugins->load($main_dir);
$plugin = $loaded_plugins->getPluginInfo($id);
$files  = $loaded_plugins->getFiles($id);
$xmls   = $loaded_plugins->getXmls($id);

// show readme text (if there was any)
$readme = "";
if (file_exists($plugin['filename'])) {
	$zip = open_file($plugin['filename']) or die("ERROR: Could not open zip file '{$plugin['filename']}' to look for readme.txt");
	$plugin_info = new PluginFile();
	$plugin_info = get_zip_info($zip, $plugin_info, $tmp_dir, $this_dir);
	if (strlen($plugin_info->get_readme()) > 0) {
		$readme = $plugin_info->get_readme();
	}
}

if (is_null($plugin)) {
	// go back to original page
	header("location: index.php");
}

// SMARTY: Assign variables
$smarty->assign( 'plugin_info', $plugin );
$smarty->assign( 'files', $files );
$smarty->assign( 'xmls', $xmls );
$smarty->assign( 'readme', $readme );

// SMARTY: Display page
$smarty->display('info.tpl');

?>