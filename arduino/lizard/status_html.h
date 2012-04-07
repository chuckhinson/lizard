const prog_uchar status_doc_prolog[] PROGMEM = 
"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\""
                 "\"http://www.w3.org/TR/html4/loose.dtd\">"
                 "<html>"
                 "<head>"
				 "<meta http-equiv=\"refresh\" content=\"10\">"
"<style type=\"text/css\">"
"body {"
"margin: 0px;"
"padding: 0px;"
"font-family: sans-serif;"
"background-color: #add8e6; }"

".header {"
"text-align: center;"
"background-color: #add8e6; }"

".columns {"
"background-color: white;"
"margin: 0px;"
"padding: 50px;"
"}"

".info-column {"
"display: inline-block;"
"font-size: larger;"
"width: 49%;"
"vertical-align: top;"
"}"

".indicator-column {"
"display: inline-block;"
"width: 49%;"
"}"

".indicator-list {"
"list-style-type: none;"
"}"

".indicator-name {"
"vertical-align: 15%;"
"font-size: xx-large;"
"margin-left: 20px;"
"}"

".indicator {"
"margin-bottom: 20px;"
"margin-left: 20px;"
"border-style: solid none solid solid;"
"display: inline-block;"
"font-size: 140%;"
"}"

".off-indicator {"
"padding: 0px 2px 1px 2px;"
"border-right-style: solid;"
"}"

".off-active {"
"background-color: #90ee90;"
"}"

".on-indicator {"
"padding: 0px 2px 1px 2px;"
"border-right-style: solid;"
"}"

".on-active {"
"background-color: red;"
"}"

".event-time {"
"padding: 2px;"
"text-align: center;"
"display: block;"
"border-top-style: solid;"
"border-right-style: solid;"
"font-size: 60%;"
"}"

"</style>"
"<title>Lizard System Status</title>"
"</head>"
"<body>"
"<div class=\"header\">"
"<h1>Lizard System Status</h1>"
"</div>"
"<div class=\"columns\">";				 

const prog_uchar status_col_line1[] PROGMEM = "<div class=\"info-column\">\n<p>Last boot: ";
const prog_uchar status_col_line2[] PROGMEM = "</p><p>Last Network Restart: ";
const prog_uchar status_col_line3[] PROGMEM = "</p><p>Number of Network Restarts: ";

const prog_uchar indicator_col_start[] PROGMEM = "<div class=\"indicator-column\">\n<ul class=indicator-list>\n";
const prog_uchar indicator_col_end[] PROGMEM = "</ul>\n</div>";

const prog_uchar indicator_line_start[] PROGMEM = "<li>\n<div class=\"indicator\">\n<span class=\"off-indicator ";
const prog_uchar indicator_line_off[] PROGMEM = "\">OFF</span><span class=\"on-indicator ";
const prog_uchar indicator_line_on[] PROGMEM = "\">ON</span><span class=\"event-time\">";
const prog_uchar indicator_line_name[] PROGMEM = "</span></div>\n<span class=\"indicator-name\">Line ";
const prog_uchar indicator_line_end[] PROGMEM = "</span>\n</li>";

const prog_uchar status_doc_epilog[] PROGMEM = "</div><body></html>";

