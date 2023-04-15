var sheet_id = '1mSnCxqtPwOg_**************_37MMunEows14'; // use your Spreadsheet ID
var sheet_url='https://docs.google.com/spreadsheets/d/1mSnCxqtPw**************Xl37MMunEows14/edit';// use your url
// var sheet = SpreadsheetApp.openById(sheet_id).getActiveSheet();
var sheet = SpreadsheetApp.openByUrl(sheet_url).getSheetByName("oxymeter");
function doGet(e) {
    var action = e.parameters.action;
    if(action=="create") return addData(e); 
    if(action =="get") return getData(e);

  }
function addData(e){
  var result="";
    var newRow = sheet.getLastRow() + 1; 
    var rowData = [];
    var Curr_Date = new Date();
    rowData[0] = Curr_Date; // Date in column A
    var Curr_Time = Utilities.formatDate(Curr_Date, "Asia/Kolkata", 'HH:mm:ss');
    rowData[1] = Curr_Time; // Time in column B
    rowData[2] = e.parameter.pulserate; // Temperature in column C
            result = 'Pulse rate Written on column C'; 
    rowData[3] = e.parameter.oxygen; // Humidity in column D
            result += ' ,oxygen level Written on column D'; 
    rowData[4] = e.parameter.temperature; // Humidity in column E
            result += ' ,tempreture Written on column E'; 

    

    Logger.log(JSON.stringify(rowData));
    var newRange = sheet.getRange(newRow, 1, 1, rowData.length);
    newRange.setValues([rowData]);

    return ContentService.createTextOutput(result).setMimeType(ContentService.MimeType.TEXT);


    }
function getData(e){
  var size = e.parameter.size;
  records={};
  data=[];
  var rows=sheet.getRange(sheet.getLastRow()-size-1, 2, sheet.getLastRow()-1, sheet.getLastColumn()).getValues();
  for(var i=0;i<rows.length;i++){
      var row=rows[i];
      var record={};
      if(row[0]=="") {break;}

      record["time"]=row[0];
      record["pulserate"]=row[1];
      record["spo2"]=row[2];
      record["temperature"]=row[3];

      data.push(record);
  }
  records.item=data;
  var result=JSON.stringify(records);
  return ContentService.createTextOutput(result).setMimeType(ContentService.MimeType.JSON);


}

function stripQuotes( value ) {
      return value.replace(/^["']|['"]$/g, "");
      }
