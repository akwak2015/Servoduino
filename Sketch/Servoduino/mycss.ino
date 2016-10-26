String cssSwitch() {
  String sReturn;
  sReturn = "  /* The switch - the box around the slider */\n";
  sReturn += ".switch {\n";
  sReturn += "  position: relative;\n";
  sReturn += "  display: inline-block;\n";
  sReturn += "  width: 30px;\n";  //60
  sReturn += "  height: 17px;\n"; //34
  sReturn += "}\n";
  
  sReturn += "/* Hide default HTML checkbox */\n";
  sReturn += ".switch input {display:none;}\n";
  
  sReturn += "/* The slider */\n";
  sReturn += ".slider {\n";
  sReturn += "  position: absolute;\n";
  sReturn += "  cursor: pointer;\n";
  sReturn += "  top: 0;\n";
  sReturn += "  left: 0;\n";
  sReturn += "  right: 0;\n";
  sReturn += "  bottom: 0;\n";
  sReturn += "  background-color: #ccc;\n";
  sReturn += "  -webkit-transition: .4s;\n";
  sReturn += "  transition: .4s;\n";
  sReturn += "}\n";
  
  sReturn += ".slider:before {\n";
  sReturn += "  position: absolute;\n";
  sReturn += "  content: \"\";\n";
  sReturn += "  height: 13px;\n";  //26
  sReturn += "  width: 13px;\n"; //26
  sReturn += "  left: 2px;\n"; //4
  sReturn += "  bottom: 2px;\n"; //4
  sReturn += "  background-color: white;\n";
  sReturn += "  -webkit-transition: .4s;\n";
  sReturn += "  transition: .4s;\n";
  sReturn += "}\n";
  
  sReturn += "input:checked + .slider {\n";
  sReturn += "  background-color: #2196F3;\n";
  sReturn += "}\n";
  
  sReturn += "input:focus + .slider {\n";
  sReturn += "  box-shadow: 0 0 1px #2196F3;\n";
  sReturn += "}\n";
  
  sReturn += "input:checked + .slider:before {\n";
  sReturn += "  -webkit-transform: translateX(13px);\n"; //26
  sReturn += "  -ms-transform: translateX(13px);\n";//26
  sReturn += "  transform: translateX(13px);\n";//26
  sReturn += "}\n";
  
  sReturn += "/* Rounded sliders */\n";
  sReturn += ".slider.round {\n";
  sReturn += "  border-radius: 17px;\n"; //34
  sReturn += "}\n";
  
  sReturn += ".slider.round:before {\n";
  sReturn += "  border-radius: 50%;\n";
  sReturn += "}\n";

  sReturn += "P { text-align: center }";

  
  return(sReturn);
}


String cssForm() {
  String sReturn = "";
  sReturn += "  H1 {\n";
  sReturn += "    background-color: lightgray;\n";
  sReturn += "    border-radius: 5px;\n";
  sReturn += "    padding: 5px;\n";
  sReturn += "    clear: both;\n";
  sReturn += "    width: 350px;\n";
  sReturn += "    }\n";
  sReturn += "  table {\n";
  sReturn += "    border-radius: 5px;\n";
  sReturn += "    padding: 5px;\n";
  sReturn += "    clear: both;\n";
  sReturn += "    }\n";
  sReturn += "  table.gray {\n";
  sReturn += "    background-color: lightgray;\n";
  sReturn += "    border-radius: 5px;\n";
  sReturn += "    padding: 5px;\n";
  sReturn += "    clear: both;\n";
  sReturn += "    }\n";
  sReturn += "  tr.hidden {\n";
  sReturn += "    background-color: #c5c5c5;\n";
  sReturn += "    padding: 5px;\n";
  sReturn += "    margin-left:50px;\n";
  sReturn += "    width:240px;\n";
  sReturn += "    clear: both;\n";
  sReturn += "    }\n";
  sReturn += "  table.fixed {table-layout:fixed; width:300px;}";
  sReturn += "  table.fixed td {overflow:hidden;}";
  // sReturn += "  table.fixed td:nth-of-type(1) {width:70px;}";
  // sReturn += "  table.fixed td:nth-of-type(2) {width:200px;}";
  sReturn += "  td.first {\n";
  sReturn += "    width: 160px;\n";
  sReturn += "    }\n";
  sReturn += "  td.firstButton {\n";
  sReturn += "    width: 40px;\n";
  sReturn += "    }\n";
  sReturn += "  td.second {\n";
  sReturn += "    width: 200px;\n";
  sReturn += "    }\n";
  sReturn += "  table.HM {\n";
  sReturn += "    background-color: #e0f0ff;\n";//lightblue
  sReturn += "    border-radius: 5px;\n";
  sReturn += "    padding: 5px;\n";
  sReturn += "    clear: both;\n";
  sReturn += "    }\n";
  sReturn += "  table.Loxone {\n";
  sReturn += "    background-color: #e0ffe0;\n"; //lightgreen
  sReturn += "    border-radius: 5px;\n";
  sReturn += "    padding: 5px;\n";
  sReturn += "    clear: both;\n";
  sReturn += "    }\n";
  sReturn += "  table.Expert {\n";
  sReturn += "    background-color: #ffe0e0;\n"; //lightgreen
  sReturn += "    border-radius: 5px;\n";
  sReturn += "    padding: 5px;\n";
  sReturn += "    clear: both;\n";
  sReturn += "    }\n";

  sReturn += "  td.on {background-color: lightgreen;}\n";
  sReturn += "  td.off {background-color: #ff8080;}\n";
  sReturn += "  td.timeron {background-color: #e2ffc6; text-align: right;}\n";
  sReturn += "  p.sys2inf {\n";
  sReturn += "    border-style: none none solid none;\n";
  sReturn += "    border-width: 1px;\n";
  sReturn += "    border-color: lightyellow;\n";
  sReturn += "    padding: 5px;\n";
  sReturn += "    width: 80%\n";
  sReturn += "    }\n";
  sReturn += "  div.all {\n";
  sReturn += "    background-color: lightgrey;\n";
  sReturn += "    border-radius: 5px;\n";
  sReturn += "    width: 350px;\n";
  sReturn += "    padding: 5px;\n";
  sReturn += "    clear: both;\n";
  sReturn += "    }\n";
  sReturn += "  p.small {";
  sReturn += "    font-size:75%;\n";
  sReturn += "    text-align: left;\n";
  sReturn += "    }\n";
  sReturn += "  input.field {width:150px;}\n";

  return sReturn;
}
