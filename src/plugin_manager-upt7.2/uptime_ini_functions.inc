<?php
include_once('plugin_functions.inc');

// UPTIME.LAX AND UptimeDataCollector.ini CLASSES
//
// uptime.lax/UptimeDataCollector.ini class
class UptimeIniFile {
	// variables
	protected $total_new_jars = 0;
	protected $uptime_dir;

	protected $file_name = "uptime.jcnf";
	protected $full_file_name = "uptime.jcnf";

	public $all_jar_files = array();	// all the jar files loaded by the core

	public function get_uptime_dir() { return $this->uptime_dir; }
	protected function set_uptime_dir($val) { $this->uptime_dir = $val; $this->update_full_file_name(); }
	public function get_file_name() { return $this->file_name; }
	public function set_file_name($val) { $this->file_name = $val; $this->update_full_file_name(); }
	
	protected function update_full_file_name() { $this->full_file_name = fix_directory_slashes( "{$this->uptime_dir}/{$this->file_name}" ); }
	public function get_full_file_name() { return $this->full_file_name; }
	
	public function get_all_jar_files() { return $this->all_jar_files; }
	public function set_all_jar_files($val) { $this->all_jar_files = $val; }


	protected function make_backup() {
		$success = false;
		if ( file_exists($this->get_full_file_name()) ) {
			// let's make 2 backups
			// uptime.lax.bak1
			// uptime.lax.bak2
			$backup1 = $this->get_full_file_name() . ".old";
			$backup2 = $this->get_full_file_name() . ".older";
			if ( file_exists($backup1) ) {
				if ( file_exists($backup2) ) {
					// delete old backup2
					unlink($backup2);
				}
				// make backup 2
				copy($backup1, $backup2);
				// delete old backup1
				unlink($backup1);
			}
			// make backup 1
			$success = copy($this->get_full_file_name(), $backup1);
			
			if ( ! $success ) {
				// error copying :(
				print("Error: Couldn't copy '" . $this->get_full_file_name() . "' to '{$backup1}'.");
			}
		}
		return $success;
	}
	
	public function jar_is_loaded($jar_file) {
		// remove any slashes that may exist at the beginning
		if ( preg_match("/^\//i", $jar_file) ) {
			$jar_file = substr($jar_file, 1);
		}
		
		if (isset($this->all_jar_files) && ! is_null($this->all_jar_files) && count($this->all_jar_files) > 0) {
			foreach ($this->all_jar_files as $cur_jar) {
				// remove any slashes that may exist at the beginning
				if ( preg_match("/^\//i", $cur_jar) ) {
					$cur_jar = substr($cur_jar, 1);
				}

				if (get_os_platform() == 'posix') {	// Posix
				
					// $jar_file is a relative path to the jar file in the plugin (e.g. /core/custom_jars/something.jar)
					// $cur_jar is the absolute classpath that's in the config file now (e.g. /usr/local/uptime/core/custom_jars/something.jar)
					// Since on Linux, we just want the directory where the jars are. Therefore:
					// test if the plugin jar, prepending uptime dir, is already listed in the config file
					if ( fix_directory_slashes(strtolower($cur_jar)) == substr(remove_file_name_from_dir( fix_directory_slashes(strtolower($this->get_uptime_dir()."/".$jar_file))),1)) {
						return true;
					}
				}
				else {
					if ( strtolower($cur_jar) == strtolower($jar_file) ) {
						return true;
					}
				}
			}
		}
		return false;
	}
	protected function add_to_all_jar_files($jar, $add_to_top = false) {
		// check if it's already been added; to remove duplicates
		if ( ! $this->jar_is_loaded($jar) ) {
			if ($add_to_top) {
				// push onto the beginning of the array
				array_unshift($this->all_jar_files, $jar);
			}
			else {
				// push onto the end of the array
				array_push($this->all_jar_files, $jar);
			}
		}
	}
	public function add_new_jar_file($jar) {
		$added = false;
		// check if it was put in the "/core/" directory and if it's a jar
		if ( preg_match("/^core\//i", strtolower($jar)) || preg_match("/^\/core\//i", strtolower($jar)) ) {
			$extension = get_file_extension($jar);
			if ($extension == 'jar') {
				if (get_os_platform() == 'posix') {	// Posix
					// add jar file
					$this->add_to_all_jar_files($jar, true);
					// add to total_new_jars count
					$this->total_new_jars++;
					$added = true;
				}

				else {	// Windows
					// since in Windows we're only tracking the directory, let's remove the filename and add the uptime dir
					// add jar directory
					$real_dir = remove_file_name_from_dir( fix_directory_slashes($this->get_uptime_dir() . "/" . $jar) );
					$this->add_to_all_jar_files($real_dir, true);
					// add to total_new_jars count
					$this->total_new_jars++;
					$added = true;
				}
			}
		}
		return $added;
	}
	public function remove_jar_file($jar_file) {
		if (get_os_platform() == 'posix') {	// Posix
			
			// remove any slashes that may exist at the beginning
			if ( preg_match("/^\//i", $jar_file) ) {
				$jar_file = substr($jar_file, 1);
			}
			
			// check if it exists first
			if ( $this->jar_is_loaded($jar_file) ) {
				for ($i = 0; $i < count($this->all_jar_files); $i++) {
					$cur_jar = $this->all_jar_files[$i];
					if ( strtolower($cur_jar) == strtolower($jar_file) ) {
						//found; now let's delete it
						unset($this->all_jar_files[$i]);
						// update the total_new_jars count
						$this->total_new_jars++;
					}
				}
			}
		}
	}
	protected function jar_line_to_string() {
		$line = "";
                $count = 0;
		
		if (get_os_platform() == 'posix') {	// Posix
			$newline = "\n";
			//$line = "lax.class.path=";
			$line = "";
/*
			$delimiter = ":";
			if ( count($this->all_jar_files) > 0 ) {
				foreach ($this->all_jar_files as $cur_jar) {
					$line .= $cur_jar . $delimiter;
				}
				$line .= $newline;
			}
*/

			if ( count($this->all_jar_files) > 0 ) {
				foreach ($this->all_jar_files as $cur_jar) {
					
					// If uptime dir is not part of the path, that means this is a jar from the plugin
					// need to add uptime dir before writing to config file
					if (strstr($cur_jar,$this->get_uptime_dir()) == FALSE ) {
						$cur_jar = remove_file_name_from_dir($this->get_uptime_dir().$cur_jar);
					} 
					
					$cur_jar = $cur_jar."/*".$newline;

					// If haven't added the jar dir to the config file yet, add it
					if(($line == "") ||strstr($cur_jar, $line)== FALSE) {
						$new_dir = fix_directory_slashes($cur_jar);
						$line .= "{$new_dir}";
						$count++;
					}
				}
			}


		}
		else {	// Windows
			$newline = "\r\n";
			// classpath.1=%UPTIMEROOT%/core/*.jar
			// classpath.2=%UPTIMEROOT%/core/custom_jars/*.jar
			$count = 1;
			if ( count($this->all_jar_files) > 0 ) {
				foreach ($this->all_jar_files as $cur_jar) {
					// convert full directory to wrapper.conf formatted dir
					$cur_jar .= "/*.jar{$newline}";
					$new_dir = fix_directory_slashes($cur_jar);
					$line .= "classpath.{$count}={$new_dir}";
					$count++;
				}
			}
		}
		return $line;
	}



	// load function
	public function load($uptime_dir) {
 		$read_the_line = false;
		$count = 0;
		$this->set_uptime_dir($uptime_dir);
		if (get_os_platform() == 'posix') {	// Posix
			// check if file exists
			if (file_exists($this->get_full_file_name())) {
				
				// read file
				$file = @fopen($this->get_full_file_name(), "r");
				if ($file) {
					while(!feof($file)) {
						// read the next line
						$line = trim(fgets($file));
						if (preg_match("/uptime.classpath/i", $line)) {
							// found the line, now let's parse it
							$read_the_line = true;
							continue;
						}
						elseif(($read_the_line) && (strlen(trim($line))!=0)) {
							$count += $this->parse_line($line);	
						}
						// End of the classpath section.  Stop reading
						elseif(($read_the_line) && (strlen(trim($line))==0)) {
							break;
						}
					}
					fclose($file);
				}
			}
		}
		else {	// Windows
			// set to use the correct file names
			$this->set_file_name("UptimeDataCollector.ini");
			
			// check if the "core/custom_jars" directory is already added
			// check if file exists
			if (file_exists($this->get_full_file_name())) {
				
				// read file
				$file = @fopen($this->get_full_file_name(), "r");
				if ($file) {
					while(!feof($file)) {
						// read the next line
						$line = trim(fgets($file));
						if (preg_match("/^classpath/i", trim($line))) {
							// found the line, now let's parse it
							$count += $this->parse_line($line);
						}
					}
					fclose($file);
				}
			}
		}
		
		return $count;
	}

	protected function parse_line($line) {
		// count the number of items found
		$count = 0;
		if (get_os_platform() == 'posix') {	// Posix
		
			$real_dir = remove_file_name_from_dir( fix_directory_slashes( $line) );

			// let's add it to the list of jars/dirs
            $this->add_to_all_jar_files($real_dir);
            $count++;
		}
		else {	// Windows
			// Lines format:
			// wrapper.java.classpath.1=%UPTIMEROOT%/core/*.jar
			// wrapper.java.classpath.2=%UPTIMEROOT%/core/websphere_mq/*.jar
			// wrapper.java.classpath.3=%UPTIMEROOT%/core/as400/*.jar
			// let's split on "="
			$arr_line = explode('=', $line, 2);
			if ( count($arr_line) == 2) {
				$dir = $arr_line[1];
				// replace %UPTIMEROOT% with the uptime directory
				$real_dir = remove_file_name_from_dir( fix_directory_slashes( $dir ) );
				
				// let's add it to the list of jars/dirs
				$this->add_to_all_jar_files($real_dir);
				$count++;
			}
		}
		return $count;
	}

	// save function
	public function save() {
		if ($this->total_new_jars == 0) {
			// no changes, so don't save anything
			return true;
		}
		
		// make backup file first
		$success = $this->make_backup();

		if (get_os_platform() == 'posix') {	// Posix
			$newline = "\n";
			// check if file exists
			if (file_exists($this->get_full_file_name())) {
				// read the contents of the current uptime.lax before and after the lax.path line
				$lax_begin = "";
				$lax_end = "";
				$add_to_end = false;
			      	$skip_next_line = false;
				
				$file = @fopen($this->get_full_file_name(), "r");
				if ($file) {
					while(!feof($file)) {
						// read the next line
						$line = trim(fgets($file));
						// Everything including and before [uptime.classpath] is added to the $lax_begin variable
						// After including [uptime.classpath], skip the rest because we will re-write them
						if (preg_match("/uptime.classpath/i", $line)) {
							$lax_begin .= $line . $newline;
							$skip_next_line = true;
							// add it to the end now
							$add_to_end = true;
							continue;
						}
						elseif (($add_to_end)&&($skip_next_line)&&(strlen(trim($line))==0))   {

							$skip_next_line = false;
							$lax_end .= $line . $newline;
							continue;
						}

						elseif (($add_to_end)&&(!$skip_next_line)) {
							// add it to the end
							$lax_end .= $line . $newline;
						}

						elseif (!($add_to_end)){
							// add it to the beginning
							$lax_begin .= $line . $newline;
						}

					}
					fclose($file);
				}
			
				if ($success) {
					// read file
					$file = @fopen($this->get_full_file_name(), "w");
					if ($file) {
						// write the contents
						fwrite($file, $lax_begin);
						fwrite($file, $this->jar_line_to_string());
						fwrite($file, $lax_end);
						$success = true;
					}
					fclose($file);
				}
			}
		}
		else {	// Windows
			$newline = "\r\n";
			// Lines format:
			// classpath.1=%UPTIMEROOT%/core/*.jar
			// classpath.2=%UPTIMEROOT%/core/custom_jars/*.jar
			
			// check if file exists
			if (file_exists($this->get_full_file_name())) {
				// read the contents of the current wrapper.conf before and after the classpath line
				$lax_begin = "";
				$lax_end = "";
				$add_to_end = false;
				
				$file = @fopen($this->get_full_file_name(), "r");
				if ($file) {
					while(!feof($file)) {
						// read the next line
						$line = trim(fgets($file));
						if (preg_match("/^classpath/i", trim($line))) {
							// got to the jars section in the file, so let's skip them all (we'll write all the jars we have instead)
							// switch so we know what to write after the jars section
							$add_to_end = true;
							continue;
						}
						elseif ($add_to_end) {
							// add it to the end
							$lax_end .= $line . $newline;
						}
						else {
							// add it to the beginning
							$lax_begin .= $line . $newline;
						}
					}
					fclose($file);
				}
				
				if ($success) {
					// read file
					$file = @fopen($this->get_full_file_name(), "w");
					if ($file) {
						// write the contents
						fwrite($file, $lax_begin);
						fwrite($file, $this->jar_line_to_string());
						fwrite($file, $lax_end);
						$success = true;
					}
					fclose($file);
				}
			}
		}
		return $success;
	}


}

?>
