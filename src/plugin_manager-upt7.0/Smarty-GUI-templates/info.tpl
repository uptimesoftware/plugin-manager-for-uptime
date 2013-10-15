{* Smarty *}
<html>
<head>
<link rel="stylesheet" href="uptime.css" type="text/css" />

</head>
<body>
<div class="TitleBar">Plugin Info - {$plugin_info['name']}</div>

<br /><a href='index.php'>Go Back</a><br /><br />

<table border='0' cellspacing="0" cellpadding="0" id="svStatusList" class='tablehelper'>
<tr class="RowDivider">
<th>Name:</th>
<td>{$plugin_info['name']} (v{$plugin_info['version']})</td>
</tr>

<tr class="RowDivider">
<th>Description:</th>
<td>{$plugin_info['desc']}</td>
</tr>

<tr class="RowDivider">
<th>Installed:</th>
<td>{$plugin_info['install_date']|date_format:"%Y-%m-%d %I:%M:%S %p"}</td>
</tr>

<tr class="RowDivider">
<th>Backup of Original Package:</th>
<td>{$plugin_info['filename']}</td>
</tr>

</table>
<br /><br />


<table border='0' cellspacing="0" cellpadding="0" id="svStatusList" class='tablehelper'>
<tr class="RowDivider">
	<th>
		Monitor Name
	</th>
	<th>
		Monitor Definition File(s)
	</th>
</tr>
{section name=num loop=$xmls step=1}
<tr>
	<td>
		{$xmls[num]['monitor']}
	</td>
	<td>
		{$xmls[num]['xml']}
	</td>
</tr>
{/section}
</table>
<br /><br />


<table border='0' cellspacing="0" cellpadding="0" id="svStatusList" class='tablehelper'>
<tr class="RowDivider">
	<th>
		File(s)/Script(s)
	</th>
</tr>
{section name=num loop=$files step=1}
<tr>
	<td>
		{$files[num]}
	</td>
</tr>
{/section}
</table>

</body></html>
