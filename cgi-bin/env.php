<?php
// Shows every environment variable the server passed to the script.
require __DIR__ . '/common.php';

$cgi_vars = ['REQUEST_METHOD', 'QUERY_STRING', 'CONTENT_LENGTH', 'CONTENT_TYPE',
    'SCRIPT_NAME', 'SCRIPT_FILENAME', 'PATH_INFO', 'SERVER_NAME', 'SERVER_PORT',
    'SERVER_PROTOCOL', 'SERVER_SOFTWARE', 'GATEWAY_INTERFACE', 'REMOTE_ADDR',
    'REDIRECT_STATUS'];

function is_cgi_var($name, $cgi_vars)
{
    return in_array($name, $cgi_vars, true) || strpos($name, 'HTTP_') === 0;
}

$vars = getenv();
$names = array_keys($vars);
usort($names, function ($a, $b) use ($cgi_vars) {
    $ca = is_cgi_var($a, $cgi_vars);
    $cb = is_cgi_var($b, $cgi_vars);
    if ($ca !== $cb)
        return $ca ? -1 : 1;
    return strcmp($a, $b);
});

$rows = '';
foreach ($names as $n) {
    $label = is_cgi_var($n, $cgi_vars) ? '<strong>' . esc($n) . '</strong>' : esc($n);
    $rows .= '<tr><td class="mono">' . $label . '</td><td class="mono">' . esc($vars[$n]) . '</td></tr>';
}

$footer = generated_by('cgi-bin/env.php');

$content = <<<HTML
<h1 class="page-title">CGI environment (PHP)</h1>
<p class="page-intro">The server gives the request to the script through these variables.
   CGI variables are in bold and listed first. Try adding <code>?foo=bar</code> to the URL.</p>
<div class="card">
    <div class="table-wrap">
        <table>
            <thead><tr><th>Variable</th><th>Value</th></tr></thead>
            <tbody>$rows</tbody>
        </table>
    </div>
</div>
$footer
HTML;

echo page('CGI environment', $content);
