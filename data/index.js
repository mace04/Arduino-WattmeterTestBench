function toggleSensor(checkboxId, pinId)
{
    if (document.getElementById(checkboxId).checked)
        {
            document.getElementById(pinId).disabled = false;
        }
        else
        {
            document.getElementById(pinId).disabled = true;
            document.getElementById(pinId).value = "";
        }
} 

function pinValue(pinId)
{
    if (document.getElementById(pinId).value == "0")
    {
        document.getElementById(pinId).value = "";
    }
}

function defaultInputStates()
{
    toggleSensor("isSensorCurr","currentSensorPin")
    toggleSensor("isSensorVFAS","vfasSensorPin")
    toggleSensor("isSensorA3","a3SensorPin")
    toggleSensor("isSensorA4","a4SensorPin")
    pinValue("smartportRxPin");
    pinValue("smartportTxPin");
}