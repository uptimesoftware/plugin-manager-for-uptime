<pre>
<?php

function test_lib($lib_name) {
	print "{$lib_name} : " . (extension_loaded($lib_name) ? "pass\n" : "fail\n");
}

// Test libraries
print "Testing Loaded Libraries\n";
test_lib("zip");

?>
</pre>
