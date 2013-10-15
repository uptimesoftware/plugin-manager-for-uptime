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

if (is_null($plugin)) {
	// go back to original page
	header("location: index.php");
}

// SMARTY: Assign variables
$smarty->assign( 'plugin_info', $plugin );
$smarty->assign( 'files', $files );
$smarty->assign( 'xmls', $xmls );

// SMARTY: Display page
$smarty->display('info.tpl');

?>