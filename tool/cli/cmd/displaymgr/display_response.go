package displaymgr

import (
	pb "kouros/api"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"code.cloudfoundry.org/bytefmt"
	"encoding/json"
	"fmt"
	"google.golang.org/protobuf/encoding/protojson"
	"google.golang.org/protobuf/reflect/protoreflect"
	"os"
	"pnconnector/src/util"
	"strconv"
	"text/tabwriter"
)

func toByte(displayUnit bool, size uint64) string {
	if displayUnit {
		return bytefmt.ByteSize(size)
	}

	return strconv.FormatUint(size, 10)
}

func PrintProtoResponse(command string, res protoreflect.ProtoMessage) error {
	m := protojson.MarshalOptions{
		EmitUnpopulated: true,
	}
	resByte, err := m.Marshal(res)
	if err != nil {
		fmt.Printf("failed to marshal the protobuf response: %v", err)
		return err
	}

	resJson := string(resByte)
	PrintResponse(command, resJson, globals.IsDebug, globals.IsJSONRes, globals.DisplayUnit)

	return nil
}

func PrintWBTResponse(command string, res protoreflect.ProtoMessage) error {
	grpcRes := res.(*pb.WBTResponse)
	grpcStatus := grpcRes.GetResult().GetStatus()
	status := messages.Status{
		CODE:        int(grpcStatus.GetCode()),
		EVENTNAME:   grpcStatus.GetEventName(),
		DESCRIPTION: grpcStatus.GetDescription(),
		CAUSE:       grpcStatus.GetCause(),
		SOLUTION:    grpcStatus.GetSolution(),
	}

	if isFailed(*grpcStatus) && !globals.IsJSONRes {
		printEventInfo(status.CODE, status.EVENTNAME,
			status.DESCRIPTION, status.CAUSE,
			status.SOLUTION)
		return nil
	}

	wbtdata := grpcRes.GetResult().GetData().GetTestdata()
	var dataString string
	var dataMap map[string]interface{}
	if len(wbtdata) > 7 {
		dataString = wbtdata[7:]
	}
	if !globals.IsJSONRes {
		fmt.Println(dataString)
		return nil
	}
	json.Unmarshal([]byte(dataString), &dataMap)
	cliRes := messages.Response{
		COMMAND: command,
		RID:     grpcRes.GetRid(),
		RESULT: messages.Result{
			STATUS: status,
		},
		INFO: messages.Info{
			VERSION: grpcRes.GetInfo().GetVersion(),
		},
	}
	if dataMap != nil {
		cliRes.RESULT.DATA = dataMap
	}
	jsonRes, err := json.Marshal(&cliRes)
	if err != nil {
		fmt.Println(err)
		return err
	}
	printResInJSON(string(jsonRes))
	return nil
}

func PrintResponse(command string, resJson string, isDebug bool, isJSONRes bool, displayUnit bool) {
	if isJSONRes {
		printResInJSON(resJson)
	} else if isDebug {
		printResToDebug(resJson)
	} else {
		printResToHumanReadable(command, resJson, displayUnit)
	}
}

func printResToDebug(resJson string) {
	res := messages.Response{}
	json.Unmarshal([]byte(resJson), &res)

	printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
		res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
	statusInfo, _ := util.GetStatusInfo(res.RESULT.STATUS.CODE)

	w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

	fmt.Fprintln(w)
	fmt.Fprintln(w, "------------ Debug message ------------")

	fmt.Fprintln(w, "Code\t: ", statusInfo.Code)
	fmt.Fprintln(w, "Level\t: ", statusInfo.Level)
	fmt.Fprintln(w, "Description\t: ", statusInfo.Description)
	fmt.Fprintln(w, "Problem\t: ", statusInfo.Problem)
	fmt.Fprintln(w, "Solution\t: ", statusInfo.Solution)
	//fmt.Fprintln(w, "Data\t: ", res.RESULT.DATA)

	w.Flush()
}

func printResInJSON(resJson string) {
	if resJson != "" {
		fmt.Println("{\"Response\":", resJson, "}")
	}
}

// TODO(mj): Currently, the output records may have whitespace.
// It should be assured that the output records do not have whitespace to pipeline the data to awk.
func printResToHumanReadable(command string, resJson string, displayUnit bool) {
	switch command {
	case "LISTARRAY":
		res := &pb.ListArrayResponse{}
		json.Unmarshal([]byte(resJson), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		// Header
		fmt.Fprintln(w,
			"Index\t"+
				globals.FieldSeparator+"Name\t"+
				globals.FieldSeparator+"Status\t"+
				globals.FieldSeparator+"DatetimeCreated\t"+
				globals.FieldSeparator+"DatetimeUpdated\t"+
				globals.FieldSeparator+"TotalCapacity\t"+
				globals.FieldSeparator+"UsedCapacity\t"+
				globals.FieldSeparator+"WriteThrough\t"+
				globals.FieldSeparator+"RAID")

		// Horizontal line
		fmt.Fprintln(w,
			"-----\t"+
				globals.FieldSeparator+"----------\t"+
				globals.FieldSeparator+"----------\t"+
				globals.FieldSeparator+"---------------------\t"+
				globals.FieldSeparator+"---------------------\t"+
				globals.FieldSeparator+"-------------\t"+
				globals.FieldSeparator+"-------------\t"+
				globals.FieldSeparator+"-------------\t"+
				globals.FieldSeparator+"----------")

		// Data
		for _, array := range res.GetResult().GetData().GetArrayList() {
			fmt.Fprint(w,
				strconv.Itoa(int(array.GetIndex()))+"\t"+
					globals.FieldSeparator+array.GetName()+"\t"+
					globals.FieldSeparator+array.GetStatus()+"\t"+
					globals.FieldSeparator+array.GetCreateDatetime()+"\t"+
					globals.FieldSeparator+array.GetUpdateDatetime()+"\t"+
					globals.FieldSeparator+toByte(displayUnit, array.GetCapacity())+"\t"+
					globals.FieldSeparator+toByte(displayUnit, array.GetUsed())+"\t"+
					globals.FieldSeparator+strconv.FormatBool(array.GetWriteThroughEnabled())+"\t"+
					globals.FieldSeparator+array.GetDataRaid())

			fmt.Fprintln(w, "")
		}
		w.Flush()

	case "ARRAYINFO":
		res := &pb.ArrayInfoResponse{}
		json.Unmarshal([]byte(resJson), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		array := res.GetResult().GetData()

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		fmt.Fprintln(w, "Name\t: ", array.GetName())
		fmt.Fprintln(w, "Index\t: ", array.GetIndex())
		fmt.Fprintln(w, "UniqueID\t: ", array.GetUniqueId())
		fmt.Fprintln(w, "State\t: "+array.GetState())
		fmt.Fprintln(w, "Situation\t: "+array.GetSituation())
		fmt.Fprintln(w, "CreateDatetime\t: "+array.GetCreateDatetime())
		fmt.Fprintln(w, "UpdateDatetime\t: "+array.GetUpdateDatetime())
		fmt.Fprintln(w, "RebuildingProgress\t: "+array.GetRebuildingProgress())
		fmt.Fprintln(w, "Total\t: "+toByte(displayUnit, array.GetCapacity()))
		fmt.Fprintln(w, "Used\t: "+toByte(displayUnit, array.GetUsed()))
		fmt.Fprintln(w, "GCMode\t: "+array.GetGcMode())
		fmt.Fprintln(w, "MetaRAID\t: "+array.GetMetaRaid())
		fmt.Fprintln(w, "DataRAID\t: "+array.GetDataRaid())
		fmt.Fprintln(w, "WriteThrough\t: "+strconv.FormatBool(array.GetWriteThroughEnabled()))
		fmt.Fprint(w, "BufferDevs\t: ")

		for _, device := range array.GetDevicelist() {
			if device.GetType() == "BUFFER" {
				fmt.Fprint(w, device.GetName()+"\t")
			}
		}
		fmt.Fprintln(w, "")

		fmt.Fprint(w, "DataDevs\t: ")

		for _, device := range array.GetDevicelist() {
			if device.GetType() == "DATA" {
				fmt.Fprint(w, device.GetName()+"\t")
			}
		}

		fmt.Fprintln(w, "")

		fmt.Fprint(w, "SpareDevs\t: ")

		for _, device := range array.GetDevicelist() {
			if device.GetType() == "SPARE" {
				fmt.Fprint(w, device.GetName()+"\t")
			}
		}

		fmt.Fprintln(w, "")

		w.Flush()

	case "LISTVOLUME":
		res := &pb.ListVolumeResponse{}
		json.Unmarshal([]byte(resJson), &res)
		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}
		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		// Header
		fmt.Fprintln(w,
			"Name\t"+
				globals.FieldSeparator+"Index\t"+
				globals.FieldSeparator+"UUID\t"+
				globals.FieldSeparator+"Total\t"+
				globals.FieldSeparator+"Remaining\t"+
				globals.FieldSeparator+"Used%\t"+
				globals.FieldSeparator+"Status\t"+
				globals.FieldSeparator+"MaxIOPS\t"+
				globals.FieldSeparator+"MaxBW\t"+
				globals.FieldSeparator+"MinIOPS\t"+
				globals.FieldSeparator+"MinBW")

		// Horizontal line
		fmt.Fprintln(w,
			"---------\t"+
				globals.FieldSeparator+"--------\t"+
				globals.FieldSeparator+"-------------------------------\t"+
				globals.FieldSeparator+"-----------------\t"+
				globals.FieldSeparator+"-----------------\t"+
				globals.FieldSeparator+"---------\t"+
				globals.FieldSeparator+"----------\t"+
				globals.FieldSeparator+"---------\t"+
				globals.FieldSeparator+"---------\t"+
				globals.FieldSeparator+"---------\t"+
				globals.FieldSeparator+"---------")

		// Data
		for _, volume := range res.GetResult().GetData().GetVolumes() {
			fmt.Fprintln(w,
				volume.GetName()+"\t"+
					globals.FieldSeparator+strconv.Itoa(int(volume.GetIndex()))+"\t"+
					globals.FieldSeparator+volume.GetUuid()+"\t"+
					globals.FieldSeparator+toByte(displayUnit, volume.GetTotal())+"\t"+
					globals.FieldSeparator+toByte(displayUnit, volume.GetRemain())+"\t"+
					globals.FieldSeparator+strconv.FormatUint(100-(volume.GetRemain()*100/volume.GetTotal()), 10)+"\t"+
					globals.FieldSeparator+volume.GetStatus()+"\t"+
					globals.FieldSeparator+strconv.FormatUint(volume.GetMaxiops(), 10)+"\t"+
					globals.FieldSeparator+strconv.FormatUint(volume.GetMaxbw(), 10)+"\t"+
					globals.FieldSeparator+strconv.FormatUint(volume.GetMiniops(), 10)+"\t"+
					globals.FieldSeparator+strconv.FormatUint(volume.GetMinbw(), 10))
		}
		w.Flush()

	case "VOLUMEINFO":
		res := &pb.VolumeInfoResponse{}
		json.Unmarshal([]byte(resJson), &res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}
		volume := res.GetResult().GetData()

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		fmt.Fprintln(w, "UUID\t: "+volume.GetUuid())
		fmt.Fprintln(w, "Name\t: "+volume.GetName())
		fmt.Fprintln(w, "TotalCapacity\t: "+toByte(displayUnit, volume.GetTotal()))
		fmt.Fprintln(w, "RemainingCapacity\t: "+toByte(displayUnit, volume.GetRemain()))
		fmt.Fprintln(w, "Used%\t: "+strconv.FormatUint(100-(volume.GetRemain()*100/volume.GetTotal()), 10))
		fmt.Fprintln(w, "Status\t: "+volume.GetStatus())
		fmt.Fprintln(w, "MaximumIOPS\t: "+strconv.FormatUint(volume.GetMaxiops(), 10))
		fmt.Fprintln(w, "MaximumBandwidth\t: "+strconv.FormatUint(volume.GetMaxbw(), 10))
		fmt.Fprintln(w, "MinimumIOPS\t: "+strconv.FormatUint(volume.GetMiniops(), 10))
		fmt.Fprintln(w, "MinimumBandwidth\t: "+strconv.FormatUint(volume.GetMinbw(), 10))
		fmt.Fprintln(w, "SubNQN\t: "+volume.GetSubnqn())
		fmt.Fprintln(w, "Array\t: "+volume.GetArrayname())

		fmt.Fprintln(w, "")

		w.Flush()

	case "LISTDEVICE":
		res := &pb.ListDeviceResponse{}
		protojson.Unmarshal([]byte(resJson), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		// Header
		fmt.Fprintln(w,
			"Name\t"+
				globals.FieldSeparator+"SerialNumber\t"+
				globals.FieldSeparator+"Address\t"+
				globals.FieldSeparator+"Class\t"+
				globals.FieldSeparator+"ModelNumber\t"+
				globals.FieldSeparator+"NUMA\t"+
				globals.FieldSeparator+"Size")

		// Horizontal line
		fmt.Fprintln(w,
			"--------------\t"+
				globals.FieldSeparator+"-------------------\t"+
				globals.FieldSeparator+"--------------\t"+
				globals.FieldSeparator+"-------------\t"+
				globals.FieldSeparator+"--------------------------\t"+
				globals.FieldSeparator+"------\t"+
				globals.FieldSeparator+"------------------")

		// Data
		for _, device := range res.GetResult().GetData().GetDevicelist() {
			fmt.Fprintln(w,
				device.Name+"\t"+
					globals.FieldSeparator+device.SerialNumber+"\t"+
					globals.FieldSeparator+device.Address+"\t"+
					globals.FieldSeparator+device.Class+"\t"+
					globals.FieldSeparator+device.ModelNumber+"\t"+
					globals.FieldSeparator+device.Numa+"\t"+
					globals.FieldSeparator+toByte(displayUnit, device.Size))
		}
		w.Flush()

	case "SMARTLOG":
		res := messages.SMARTLOGResponse{}
		json.Unmarshal([]byte(resJson), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)
		fmt.Fprintln(w, "AvailableSpareSpace\t:", res.RESULT.DATA.AVAILABLESPARESPACE)
		fmt.Fprintln(w, "Temperature\t:", res.RESULT.DATA.TEMPERATURE)
		fmt.Fprintln(w, "DeviceReliability\t:", res.RESULT.DATA.DEVICERELIABILITY)
		fmt.Fprintln(w, "ReadOnly\t:", res.RESULT.DATA.READONLY)
		fmt.Fprintln(w, "VolatileMemoryBackup\t:", res.RESULT.DATA.VOLATILEMEMORYBACKUP)
		fmt.Fprintln(w, "CurrentTemperature\t:", res.RESULT.DATA.CURRENTTEMPERATURE)
		fmt.Fprintln(w, "AvailableSpare\t:", res.RESULT.DATA.AVAILABLESPARE)
		fmt.Fprintln(w, "AvailableSpareThreshold\t:", res.RESULT.DATA.AVAILABLESPARETHRESHOLD)
		fmt.Fprintln(w, "LifePercentageUsed\t:", res.RESULT.DATA.LIFEPERCENTAGEUSED)
		fmt.Fprintln(w, "DataUnitsRead\t:", res.RESULT.DATA.DATAUNITSREAD)
		fmt.Fprintln(w, "DataUnitsWritten\t:", res.RESULT.DATA.DATAUNITSWRITTEN)
		fmt.Fprintln(w, "HostReadCommands\t:", res.RESULT.DATA.HOSTREADCOMMANDS)
		fmt.Fprintln(w, "HostWriteCommands\t:", res.RESULT.DATA.HOSTWRITECOMMANDS)
		fmt.Fprintln(w, "ControllerBusyTime\t:", res.RESULT.DATA.CONTROLLERBUSYTIME)
		fmt.Fprintln(w, "PowerCycles\t:", res.RESULT.DATA.POWERCYCLES)
		fmt.Fprintln(w, "PowerOnHours\t:", res.RESULT.DATA.POWERONHOURS)
		fmt.Fprintln(w, "UnsafeShutdowns\t:", res.RESULT.DATA.UNSAFESHUTDOWNS)
		fmt.Fprintln(w, "unrecoverableMediaErrors\t:", res.RESULT.DATA.UNRECOVERABLEMEDIAERROS)
		fmt.Fprintln(w, "LifetimeErrorLogEntries\t:", res.RESULT.DATA.LIFETIMEERRORLOGENTRIES)
		fmt.Fprintln(w, "WarningTemperatureTime\t:", res.RESULT.DATA.WARNINGTEMPERATURETIME)
		fmt.Fprintln(w, "CriticalTemperatureTime\t:", res.RESULT.DATA.CRITICALTEMPERATURETIME)
		fmt.Fprintln(w, "TemperatureSensor1\t:", res.RESULT.DATA.TEMPERATURESENSOR1)
		fmt.Fprintln(w, "TemperatureSensor2\t:", res.RESULT.DATA.TEMPERATURESENSOR2)
		fmt.Fprintln(w, "TemperatureSensor3\t:", res.RESULT.DATA.TEMPERATURESENSOR3)
		fmt.Fprintln(w, "TemperatureSensor4\t:", res.RESULT.DATA.TEMPERATURESENSOR4)
		fmt.Fprintln(w, "TemperatureSensor5\t:", res.RESULT.DATA.TEMPERATURESENSOR5)
		fmt.Fprintln(w, "TemperatureSensor6\t:", res.RESULT.DATA.TEMPERATURESENSOR6)
		fmt.Fprintln(w, "TemperatureSensor7\t:", res.RESULT.DATA.TEMPERATURESENSOR7)
		fmt.Fprintln(w, "TemperatureSensor8\t:", res.RESULT.DATA.TEMPERATURESENSOR8)

		w.Flush()

	case "GETLOGLEVEL":
		res := &pb.GetLogLevelResponse{}
		json.Unmarshal([]byte(resJson), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		fmt.Println("Log level: " + res.GetResult().GetData().Level)

	case "LOGGERINFO":
		res := &pb.LoggerInfoResponse{}
		json.Unmarshal([]byte(resJson), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		data := res.GetResult().GetData()
		fmt.Fprintln(w, "minorLogPath\t: "+data.MinorLogPath)
		fmt.Fprintln(w, "majorLogPath\t: "+data.MajorLogPath)
		fmt.Fprintln(w, "logfileSizeInMb\t: "+data.LogfileSizeInMb)
		fmt.Fprintln(w, "logfileRotationCount\t:", data.LogfileRotationCount)
		fmt.Fprintln(w, "minAllowableLogLevel\t: "+data.MinAllowableLogLevel)
		fmt.Fprintln(w, "filterEnabled\t:", data.FilterEnabled == 1)
		fmt.Fprintln(w, "filterIncluded\t: "+data.FilterIncluded)
		fmt.Fprintln(w, "filterExcluded\t: "+data.FilterExcluded)
		fmt.Fprintln(w, "structuredLogging\t: "+strconv.FormatBool(data.StructuredLogging))

		w.Flush()

	case "CREATEQOSVOLUMEPOLICY":
		res := messages.Response{}
		json.Unmarshal([]byte(resJson), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

	case "RESETQOSVOLUMEPOLICY":
		res := messages.Response{}
		json.Unmarshal([]byte(resJson), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

	case "LISTQOSPOLICIES":
		res := &pb.ListQOSPolicyResponse{}
		protojson.Unmarshal([]byte(resJson), res)
		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}
		for _, qosResult := range res.GetResult().GetData().GetQosresult() {
			for _, array := range qosResult.GetArrayName() {
				fmt.Println("Array Name: " + array.ArrayName)
			}
			fmt.Println("")
			for _, rebuild := range qosResult.GetRebuildPolicy() {
				fmt.Println("Rebuild Impact: " + rebuild.Rebuild)
			}
			for _, volume := range qosResult.GetVolumePolicies() {
				fmt.Println("Name: " + volume.Name)
				fmt.Println("ID: ", volume.Id)
				fmt.Println("Minimum Iops: ", volume.Miniops)
				fmt.Println("Maximum Iops: ", volume.Maxiops)
				fmt.Println("Minimum Bw: ", volume.Minbw)
				fmt.Println("Maximum Bw: ", volume.Maxbw)
				fmt.Println("Minimum Bw Guarantee: " + volume.MinBwGuarantee)
				fmt.Println("Minimum IOPS Guarantee: " + volume.MinIopsGuarantee)
			}

		}
		fmt.Println("")

	case "LISTSUBSYSTEM":
		res := &pb.ListSubsystemResponse{}
		protojson.Unmarshal([]byte(resJson), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		fmt.Fprintln(w,
			"Name\t"+
				globals.FieldSeparator+"Subtype\t"+
				globals.FieldSeparator+"AddressCount\t"+
				globals.FieldSeparator+"SerialNumber(SN)\t"+
				globals.FieldSeparator+"ModelNumber(MN)\t"+
				globals.FieldSeparator+"NamespaceCount")

		// Horizontal line
		fmt.Fprintln(w,
			"-------------------------------------\t"+
				globals.FieldSeparator+"-----------\t"+
				globals.FieldSeparator+"------------\t"+
				globals.FieldSeparator+"---------------------\t"+
				globals.FieldSeparator+"---------------------\t"+
				globals.FieldSeparator+"--------------")

		// Data
		for _, subsystem := range res.GetResult().GetData().GetSubsystemlist() {
			fmt.Fprintln(w,
				subsystem.GetNqn()+"\t"+
					globals.FieldSeparator+subsystem.GetSubtype()+"\t"+
					globals.FieldSeparator+strconv.Itoa(len(subsystem.GetListenAddresses()))+"\t"+
					globals.FieldSeparator+subsystem.GetSerialNumber()+"\t"+
					globals.FieldSeparator+subsystem.GetModelNumber()+"\t"+
					globals.FieldSeparator+strconv.Itoa(len(subsystem.GetNamespaces())))
		}
		w.Flush()

	case "LISTLISTENER":
		res := &pb.ListListenerResponse{}
		protojson.Unmarshal([]byte(resJson), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		fmt.Fprintln(w,
			"Addr.trtype\t"+
				globals.FieldSeparator+"Addr.adrfam\t"+
				globals.FieldSeparator+"Addr.traddr\t"+
				globals.FieldSeparator+"Addr.trsvcid\t"+
				globals.FieldSeparator+"ana_state")

		// Horizontal line
		fmt.Fprintln(w,
			"------------\t"+
				globals.FieldSeparator+"------------\t"+
				globals.FieldSeparator+"------------------\t"+
				globals.FieldSeparator+"--------------\t"+
				globals.FieldSeparator+"---------------")

		// Data
		for _, listener := range res.GetResult().GetData().GetListenerlist() {
			fmt.Fprintln(w,
				listener.GetAddress().GetTrtype()+"\t"+
					globals.FieldSeparator+listener.GetAddress().GetAdrfam()+"\t"+
					globals.FieldSeparator+listener.GetAddress().GetTraddr()+"\t"+
					globals.FieldSeparator+listener.GetAddress().GetTrsvcid()+"\t"+
					globals.FieldSeparator+listener.GetAnaState())
		}
		w.Flush()

	case "LISTTRANSPORT":
		res := &pb.ListTransportResponse{}
		protojson.Unmarshal([]byte(resJson), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		fmt.Fprintln(w,
			"TR type\t"+
				globals.FieldSeparator+"max QD\t"+
				globals.FieldSeparator+"max IO Qpairs/ctrlr\t"+
				globals.FieldSeparator+"incapsule data sz\t"+
				globals.FieldSeparator+"max IO sz\t"+
				globals.FieldSeparator+"IO unit sz\t"+
				globals.FieldSeparator+"abort TO sec\t"+
				globals.FieldSeparator+"buf cache sz\t"+
				globals.FieldSeparator+"num shared buf")

		// Horizontal line
		fmt.Fprintln(w,
			"-------\t"+
				globals.FieldSeparator+"------\t"+
				globals.FieldSeparator+"-------------------\t"+
				globals.FieldSeparator+"-----------------\t"+
				globals.FieldSeparator+"---------\t"+
				globals.FieldSeparator+"----------\t"+
				globals.FieldSeparator+"------------\t"+
				globals.FieldSeparator+"------------\t"+
				globals.FieldSeparator+"--------------")

		// Data
		for _, transport := range res.GetResult().GetData().GetTransportlist() {
			fmt.Fprintln(w,
				transport.GetType()+"\t"+
					globals.FieldSeparator+strconv.FormatInt(int64(transport.GetMaxQueueDepth()), 10)+"\t"+
					globals.FieldSeparator+strconv.FormatInt(int64(transport.GetMaxIoQpairsPerCtrlr()), 10)+"\t"+
					globals.FieldSeparator+strconv.FormatInt(int64(transport.GetInCapsuleDataSize()), 10)+"\t"+
					globals.FieldSeparator+strconv.FormatInt(int64(transport.GetMaxIoSize()), 10)+"\t"+
					globals.FieldSeparator+strconv.FormatInt(int64(transport.GetIoUnitSize()), 10)+"\t"+
					globals.FieldSeparator+strconv.FormatInt(int64(transport.GetAbortTimeoutSec()), 10)+"\t"+
					globals.FieldSeparator+strconv.FormatInt(int64(transport.GetBufCacheSize()), 10)+"\t"+
					globals.FieldSeparator+strconv.FormatInt(int64(transport.GetNumSharedBuf()), 10))
		}
		w.Flush()

	case "SUBSYSTEMINFO":
		res := &pb.SubsystemInfoResponse{}
		protojson.Unmarshal([]byte(resJson), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		if len(res.GetResult().GetData().GetSubsystemlist()) != 0 {
			subsystem := res.GetResult().GetData().GetSubsystemlist()[0]

			w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

			fmt.Fprintln(w, "nqn\t: "+subsystem.GetNqn())
			fmt.Fprintln(w, "subtype\t: "+subsystem.GetSubtype())
			fmt.Fprint(w, "listen_addresses\t: ")
			for _, address := range subsystem.GetListenAddresses() {
				fmt.Fprintln(w, "")
				fmt.Fprintln(w, "\t  {")
				fmt.Fprintln(w, "\t    trtype : "+address.GetTransportType())
				fmt.Fprintln(w, "\t    adrfam : "+address.GetAddressFamily())
				fmt.Fprintln(w, "\t    traddr : "+address.GetTargetAddress())
				fmt.Fprintln(w, "\t    trsvcid : "+address.GetTransportServiceId())
				fmt.Fprint(w, "\t  }")
			}
			fmt.Fprintln(w, "")
			fmt.Fprintln(w, "allow_any_host\t:", subsystem.GetAllowAnyHost() != 0)
			fmt.Fprintln(w, "hosts\t: ")
			for _, host := range subsystem.GetHosts() {
				fmt.Fprintln(w, "\t  { nqn : "+host.GetNqn()+" }")
			}
			if "NVMe" == subsystem.GetSubtype() {
				fmt.Fprintln(w, "serial_number\t: "+subsystem.GetSerialNumber())
				fmt.Fprintln(w, "model_number\t: "+subsystem.GetModelNumber())
				fmt.Fprintln(w, "max_namespaces\t:", subsystem.GetMaxNamespaces())
				fmt.Fprint(w, "namespaces\t: ")
				for _, namespace := range subsystem.GetNamespaces() {
					fmt.Fprintln(w, "")
					fmt.Fprintln(w, "\t  {")
					fmt.Fprintln(w, "\t    nsid :", namespace.GetNsid())
					fmt.Fprintln(w, "\t    bdev_name : "+namespace.GetBdevName())
					fmt.Fprintln(w, "\t    uuid : "+namespace.GetUuid())
					fmt.Fprint(w, "\t  }")
				}
				fmt.Fprintln(w, "")
			}
			w.Flush()
		}

	case "LISTNODE":
		res := &pb.ListNodeResponse{}
		protojson.Unmarshal([]byte(resJson), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		// Header
		fmt.Fprintln(w,
			"Name\t"+
				globals.FieldSeparator+"IP\t"+
				globals.FieldSeparator+"Lastseen\t")

		// Horizontal line
		fmt.Fprintln(w,
			"-------------------\t"+
				globals.FieldSeparator+"-------------------\t"+
				globals.FieldSeparator+"-------------------\t")

		// Data
		for _, node := range res.GetResult().GetData() {
			fmt.Fprintln(w,
				node.Name+"\t"+
					globals.FieldSeparator+node.Ip+"\t"+
					globals.FieldSeparator+node.Lastseen+"\t")
		}
		w.Flush()

	case "LISTHAVOLUME":
		res := &pb.ListHaVolumeResponse{}
		protojson.Unmarshal([]byte(resJson), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		// Header
		fmt.Fprintln(w,
			"Id\t"+
				globals.FieldSeparator+"Name\t"+
				globals.FieldSeparator+"NodeName\t"+
				globals.FieldSeparator+"ArrayName\t"+
				globals.FieldSeparator+"Size\t"+
				globals.FieldSeparator+"Lastseen\t")

		// Horizontal line
		fmt.Fprintln(w,
			"-------\t"+
				globals.FieldSeparator+"-------------------\t"+
				globals.FieldSeparator+"-------------------\t"+
				globals.FieldSeparator+"-------------------\t"+
				globals.FieldSeparator+"-------------------\t"+
				globals.FieldSeparator+"-------------------\t")

		// Data
		for _, volume := range res.GetResult().GetData() {
			fmt.Fprintln(w,
				strconv.FormatInt(int64(volume.Id), 10)+"\t"+
					globals.FieldSeparator+volume.Name+"\t"+
					globals.FieldSeparator+volume.NodeName+"\t"+
					globals.FieldSeparator+volume.ArrayName+"\t"+
					globals.FieldSeparator+strconv.FormatInt(volume.Size, 10)+"\t"+
					globals.FieldSeparator+volume.Lastseen+"\t")
		}
		w.Flush()

	case "LISTHAREPLICATION":
		res := &pb.ListHaReplicationResponse{}
		protojson.Unmarshal([]byte(resJson), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		// Header
		fmt.Fprintln(w,
			"Id\t"+
				globals.FieldSeparator+"SourceVolumeId\t"+
				globals.FieldSeparator+"SourceWalVolumeId\t"+
				globals.FieldSeparator+"DestinationVolumeId\t"+
				globals.FieldSeparator+"DestinationWalVolumeId\t")

		// Horizontal line
		fmt.Fprintln(w,
			"-------\t"+
				globals.FieldSeparator+"---------------------------\t"+
				globals.FieldSeparator+"---------------------------\t"+
				globals.FieldSeparator+"---------------------------\t"+
				globals.FieldSeparator+"---------------------------\t")

		// Data
		for _, replication := range res.GetResult().GetData() {
			fmt.Fprintln(w,
				strconv.FormatInt(int64(replication.Id), 10)+"\t"+
					globals.FieldSeparator+strconv.FormatInt(int64(replication.SourceVolumeId), 10)+"\t"+
					globals.FieldSeparator+strconv.FormatInt(int64(replication.SourceWalVolumeId), 10)+"\t"+
					globals.FieldSeparator+strconv.FormatInt(int64(replication.DestinationVolumeId), 10)+"\t"+
					globals.FieldSeparator+strconv.FormatInt(int64(replication.DestinationWalVolumeId), 10)+"\t")
		}
		w.Flush()

	case "SYSTEMINFO":
		res := &pb.SystemInfoResponse{}
		protojson.Unmarshal([]byte(resJson), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		fmt.Fprintln(w, "PosVersion\t: "+res.GetResult().GetData().Version)
		fmt.Fprintln(w, "BiosVersion\t: "+res.GetResult().GetData().BiosVersion)
		fmt.Fprintln(w, "BiosVendor\t: "+res.GetResult().GetData().BiosVendor)
		fmt.Fprintln(w, "BiosReleaseDate\t: "+res.GetResult().GetData().BiosReleaseDate)

		fmt.Fprintln(w, "SystemManufacturer\t: "+res.GetResult().GetData().SystemManufacturer)
		fmt.Fprintln(w, "SystemProductName\t: "+res.GetResult().GetData().SystemProductName)
		fmt.Fprintln(w, "SystemSerialNumber\t: "+res.GetResult().GetData().SystemSerialNumber)
		fmt.Fprintln(w, "SystemUuid\t: "+res.GetResult().GetData().SystemUuid)

		fmt.Fprintln(w, "BaseboardManufacturer\t: "+res.GetResult().GetData().BaseboardManufacturer)
		fmt.Fprintln(w, "BaseboardProductName\t: "+res.GetResult().GetData().BaseboardProductName)
		fmt.Fprintln(w, "BaseboardSerialNumber\t: "+res.GetResult().GetData().BaseboardSerialNumber)
		fmt.Fprintln(w, "BaseboardVersion\t: "+res.GetResult().GetData().BaseboardVersion)

		fmt.Fprintln(w, "ProcessorManufacturer\t: "+res.GetResult().GetData().ProcessorManufacturer)
		fmt.Fprintln(w, "ProcessorVersion\t: "+res.GetResult().GetData().ProcessorVersion)
		fmt.Fprintln(w, "ProcessorFrequency\t: "+res.GetResult().GetData().ProcessorFrequency)

		w.Flush()

	case "GETSYSTEMPROPERTY":
		res := messages.POSPropertyResponse{}
		json.Unmarshal([]byte(resJson), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)
		fmt.Fprintln(w, "RebuildPerfImpact\t: "+res.RESULT.DATA.REBUILDPOLICY)

		w.Flush()
	case "GETTELEMETRYPROPERTY":
		res := &pb.GetTelemetryPropertyResponse{}
		protojson.Unmarshal([]byte(resJson), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		fmt.Fprintln(w, "Status\t: "+strconv.FormatBool(res.GetResult().GetData().Status))
		fmt.Fprintln(w, "PublicationListPath\t: "+res.GetResult().GetData().PublicationListPath)

		w.Flush()

	case "STARTPOS":
		res := messages.Response{}
		json.Unmarshal([]byte(resJson), &res)
		fmt.Println(res.RESULT.STATUS.DESCRIPTION)

	case "STOPPOS":
		res := messages.Response{}
		json.Unmarshal([]byte(resJson), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

		fmt.Println("PoseidonOS termination has been requested. PoseidonOS will be terminated soon.")

	case "LISTWBT":
		res := &pb.ListWBTResponse{}
		protojson.Unmarshal([]byte(resJson), res)

		status := res.GetResult().GetStatus()
		if isFailed(*status) {
			printEvent(*status)
			return
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)

		for _, test := range res.GetResult().GetData().GetTestlist() {
			fmt.Fprintln(w, test.GetTestname())
		}
		w.Flush()

	default:
		res := messages.Response{}
		json.Unmarshal([]byte(resJson), &res)

		if res.RESULT.STATUS.CODE != globals.CliServerSuccessCode {
			printEventInfo(res.RESULT.STATUS.CODE, res.RESULT.STATUS.EVENTNAME,
				res.RESULT.STATUS.DESCRIPTION, res.RESULT.STATUS.CAUSE, res.RESULT.STATUS.SOLUTION)
			return
		}

	}
}

func isFailed(status pb.Status) bool {
	return int(status.GetCode()) != globals.CliServerSuccessCode
}

func printEvent(status pb.Status) {
	code := status.GetCode()
	name := status.GetEventName()
	desc := status.GetDescription()
	cause := status.GetCause()
	solution := status.GetSolution()

	w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)
	fmtStr := "%s (%d) - %s\n"

	if cause != "" {
		fmtStr += "Cause: " + cause + "\n"
	}
	if solution != "" {
		fmtStr += "Solution: " + solution + "\n"
	}

	fmt.Fprintf(w, fmtStr, name, code, desc)
	w.Flush()
}

func printEventInfo(code int, name string, desc string, cause string, solution string) {
	w := tabwriter.NewWriter(os.Stdout, 0, 0, 1, ' ', 0)
	fmtStr := "%s (%d) - %s\n"

	if cause != "" {
		fmtStr += "Cause: " + cause + "\n"
	}

	if solution != "" {
		fmtStr += "Solution: " + solution + "\n"
	}

	fmt.Fprintf(w, fmtStr, name, code, desc)

	w.Flush()
}
