<?php

include_once('plugin_functions.inc');
include_once('rcs_function.php');

function start_uptime_core($agent_password, $plugins_dir) {
	return manage_uptime_core($agent_password, $plugins_dir, true);
}
function stop_uptime_core($agent_password, $plugins_dir) {
	return manage_uptime_core($agent_password, $plugins_dir, false);
}

function restart_uptime_core($agent_password, $plugins_dir) {
	$sleep = 3;		// seconds to sleep between stop/start
	stop_uptime_core($agent_password, $plugins_dir);
	sleep($sleep);
	return start_uptime_core($agent_password, $plugins_dir);
}

function manage_uptime_core($agent_password, $plugins_dir, $start_core = true) {
	$rv = true;
	if (get_os_platform() == 'win') {
		$host = "127.0.0.1";
		$port = 9998;
		$timeout = 10;
		
		if ($start_core) {
			$svc_cmd = "svc_start";
		}
		else {
			$svc_cmd = "svc_stop";
		}
		$cmd = "{$svc_cmd} \"up.time Data Collector\" \"{$agent_password}\"";
		$output = agentcmd($host, $port, $cmd, $timeout);
		// look for any error(s) in output
		if ( strpos(strtolower(" " . $output), "error") > 0 ) {
			// error found
			$rv = false;
		}
	}
	else {	// posix
		$new_status;
		if ($start_core) {
			$new_status = "start";
		}
		else {
			$new_status = "stop";
		}
		
		// figure out if running on Linux or Solaris
		$this_platform = php_uname("s");
		$os = "linux";
		if (strripos(" " . $this_platform, 'SunOS') > 0 || strripos(" " . $this_platform, 'Unix') > 0) {
			$os = 'solaris';
		}
		elseif (strripos(" " . $this_platform, 'Linux') > 0) {
			$os = 'linux';
		}
		
		$cmd = fix_directory_slashes("{$plugins_dir}/bin/restart_core-{$os}.bin {$new_status}");
		$last_line = exec($cmd, $exec_output, $cmd_rv);
		if ($cmd_rv == 0) {
			$rv = true;
		}
		else {
			$rv = false;
		}
	}
	return $rv;
}
?>
