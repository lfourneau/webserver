<?php
// Small calculator: a, b, op (add|sub|mul|div), from GET or POST.
// With format=text it prints a single line instead of a full page.
require __DIR__ . '/common.php';

$symbols = ['add' => '+', 'sub' => '-', 'mul' => '×', 'div' => '÷'];

[$fields] = cgi_params();
$op = $fields['op'] ?? 'add';
$a_raw = $fields['a'] ?? '0';
$b_raw = $fields['b'] ?? '0';

function fmt($x)
{
    // Same look as Python's {:g}: no trailing ".0"
    return rtrim(rtrim(sprintf('%.6F', $x), '0'), '.');
}

if (!is_numeric($a_raw) || !is_numeric($b_raw)) {
    $answer = 'Error: a and b must be numbers';
} elseif (!isset($symbols[$op])) {
    $answer = "Error: unknown operation '$op'";
} elseif ($op === 'div' && (float)$b_raw == 0) {
    $answer = 'Error: division by zero';
} else {
    $a = (float)$a_raw;
    $b = (float)$b_raw;
    switch ($op) {
        case 'add': $r = $a + $b; break;
        case 'sub': $r = $a - $b; break;
        case 'mul': $r = $a * $b; break;
        default:    $r = $a / $b; break;
    }
    $answer = fmt($a) . ' ' . $symbols[$op] . ' ' . fmt($b) . ' = ' . fmt($r);
}

if (($fields['format'] ?? '') === 'text') {
    echo "[php] $answer  (method: " . cgi_method() . ")\n";
} else {
    $ans = esc($answer);
    $footer = generated_by('cgi-bin/calc.php');
    $content = <<<HTML
<section class="hero"><h1 style="font-family: var(--mono);">$ans</h1>
<p>Computed by PHP.</p></section>
$footer
<p><a class="btn secondary" href="/cgi.html">Back to CGI page</a></p>
HTML;
    echo page('Calculator', $content);
}
