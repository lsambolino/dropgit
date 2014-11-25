/**
 * 
 */
function SPsS(k_) {
	var k = k_;
	var StandbyPowerSaving, MaxWakeUpTime, MaxSleepTime, WakeupTriggers, SleepTimer;
};

function PPsS(j_, MinPowerGain_, MaxPktThroughput_, MaxBitThroughput_) {
	var j = j_;
	var MinPowerGain = MinPowerGain_;
	var MaxPktThroughput = MaxPktThroughput_;
	var MaxBitThroughput = MaxBitThroughput_;
	var LPIPowerGain, LPIWakeupTime, LPISleepingTime, LPITransPower;
	var AutonomicSteps, AutonomicCurves, AutonomicServiceInterruptTime, 
		PPsSTransTime, PPsSInterruptTime;
};

PPsS.prototype.setAuonomicProfile = function(curves) {
	this.AutonomicSteps = curves.length;
	this.AutonomicCurves = curves;
}; 

function EAS(name_tmp, k, j, MinPowerGain, MaxPktThroughput, MaxBitThroughput) {
	var name = name_tmp;
	var description;
	var SPsS = new SPsS(k);
	var PPsS = new PPsS(j, MinPowerGain, MaxPktThroughput, MaxBitThroughput);
	
};