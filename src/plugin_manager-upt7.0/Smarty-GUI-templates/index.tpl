{* Smarty *}
<html>
<head>
<link rel="stylesheet" href="uptime.css" type="text/css" />
<style type="text/css">
upload {
	font-family: Verdana, Arial, Helvetica, sans-serif;
	font-size: 9px;
	color: #565E6C;
}
</style>

<script type="text/javascript">
// hide and rename submit button to prevent double-clicks (and look cooler ;) )
function hide_submit_button() {
	document.getElementById('click_once').value = "Uploading...";
	document.getElementById('click_once').disabled = true;
	document.getElementById('install_form').submit();
	return true;
}
</script>

</head>
<body>
<div class="TitleBar">Manage Plugins - up.time Plugin Manager v{$plugin_manager_version}</div>

<p>
	<a href="http://support.uptimesoftware.com/the-grid/" target="search">
		<img src="images/download.png" height="40" width="40" alt="Click here to download more plugins." align="left" border="0" />
		Click here to download more plugins from <b>The Grid</b>.
	</a>
	<form action="http://support.uptimesoftware.com/the-grid/search.php" method="GET" target="search">
	<input type="text" name="searchterm" value="" />
	<input type="submit" name="search" value="Search for Plugins" />
	</form>
</p>
<br />
<p>Install new plugin:
<form enctype="multipart/form-data" method="POST" id="install_form">
<input type="hidden" name="MAX_FILE_SIZE" value="75000000" />
<input type="file" name="uploadedfile" size="1000px" />
<input type="submit" value="Upload File" id="click_once" onclick="hide_submit_button();" />
</form>
</p><br/>

<table border='0' cellspacing="0" cellpadding="0" id="svStatusList" class='tablehelper'>
<tr class="RowDivider">
	<td>
		Uninstall
	</td>
	<td>
		Install Date
	</td>
	<td>
		Version
	</td>
	<td>
		Plugin Name
	</td>
	<td>
		Description
	</td>
	<td>
		More Info
	</td>
</tr>
{assign var="desc_length" value=40}
<tr>
{section name=num loop=$loaded_plugins step=1}
	<td>
		<a href='delete_plugin.php?id={$loaded_plugins[num]['id']}'><img src='images/icon-trash.gif' alt="Delete" title="Delete"
		width="25" height="20" border="0"></a>
	</td>
	<td>
		{$loaded_plugins[num]['install_date']|date_format:"%Y-%m-%d %I:%M:%S %p"}
	</td>
	<td>
		{$loaded_plugins[num]['version']}
	</td>
	<td>
		<a href="info.php?id={$loaded_plugins[num]['id']}">{$loaded_plugins[num]['name']}</a>
	</td>
	<td>
		{$loaded_plugins[num]['desc']|substr:0:$desc_length}...
	</td>
	<td>
		<a href="info.php?id={$loaded_plugins[num]['id']}">More Info...</a>
	</td>
</tr>
{/section}
</table>

</body></html>
