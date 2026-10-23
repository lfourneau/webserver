<?php
// Greets the user. Works with GET (query string) and POST (form body).
require __DIR__ . '/common.php';

[$fields, $body] = cgi_params();
$name = trim($fields['name'] ?? '');
if ($name === '')
    $name = 'stranger';

$rows = '';
foreach ($fields as $k => $v)
    $rows .= '<tr><td class="mono">' . esc($k) . '</td><td>' . esc(is_array($v) ? implode(', ', $v) : $v) . '</td></tr>';
if ($rows === '')
    $rows = '<tr><td colspan="2" class="muted">No fields received.</td></tr>';

$method = cgi_method();
$m = strtolower($method);
$qs = cgi_query_string() !== '' ? esc(cgi_query_string()) : '<span class="muted">(empty)</span>';
$b = $body !== '' ? esc($body) : '<span class="muted">(empty)</span>';
$n = esc($name);
$footer = generated_by('cgi-bin/hello.php');

$content = <<<HTML
<section class="hero">
    <h1>Hello, $n!</h1>
    <p>This page was written by a PHP script, not read from a file.</p>
</section>

<div class="grid-2">
    <div class="card">
        <h2>What the script received</h2>
        <table>
            <tbody>
                <tr><td>Method</td><td><span class="badge $m">$method</span></td></tr>
                <tr><td>Query string</td><td class="mono">$qs</td></tr>
                <tr><td>Body</td><td class="mono">$b</td></tr>
            </tbody>
        </table>
    </div>
    <div class="card">
        <h2>Parsed fields</h2>
        <table>
            <thead><tr><th>Name</th><th>Value</th></tr></thead>
            <tbody>$rows</tbody>
        </table>
    </div>
</div>
$footer
<p><a class="btn secondary" href="/get.html">GET form</a> <a class="btn secondary" href="/post.html">POST form</a></p>
HTML;

echo page("Hello $name", $content);
