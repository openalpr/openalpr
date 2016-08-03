package openalpr

/*
#cgo pkg-config: openalpr
#include <openalprgo.h>
#include <stdlib.h>
*/
import "C"
import (
	"encoding/json"
	"fmt"
	"unsafe"
)

type Alpr struct {
	//Country    string
	//configFile string
	//runtimeDir string

	cAlpr C.Alpr
}

type AlprResults struct {
	EpochTime             int64                  `json:"epoch_time"`
	ImgWidth              int                    `json:"img_width"`
	ImgHeight             int                    `json:"img_height"`
	TotalProcessingTimeMs float32                `json:"processing_time_ms"`
	Plates                []AlprPlateResult      `json:"results"`
	RegionsOfInterest     []AlprRegionOfInterest `json:"regions_of_interest"`
}

type AlprPlate struct {
	Characters        string  `json:"plate"`
	OverallConfidence float32 `json:"confidence"`
	MatchesTemplate   bool
}

type AlprRegionOfInterest struct {
	X      int `json:"x"`
	Y      int `json:"y"`
	Width  int `json:"width"`
	Height int `json:"height"`
}

type AlprCoordinate struct {
	X int `json:"x"`
	Y int `json:"y"`
}

type AlprPlateResult struct {
	RequestedTopN    int              `json:"requested_topn"`
	BestPlate        string           `json:"plate"`
	TopNPlates       []AlprPlate      `json:"candidates"`
	ProcessingTimeMs float32          `json:"processing_time_ms"`
	PlatePoints      []AlprCoordinate `json:"coordinates"`
	PlateIndex       int              `json:"plate_index"`
	RegionConfidence int              `json:"region_confidence"`
	Region           string           `json:"region"`
}

func bool2Cint(b bool) C.int {
	if b {
		return 1
	} else {
		return 0
	}
}

func cint2Bool(i C.int) bool {
	if i == 0 {
		return false
	} else {
		return true
	}
}

func NewAlpr(country string, configFile string, runtimeDir string) *Alpr {
	cstrCountry := C.CString(country)
	cstrConfigFile := C.CString(configFile)
	cstrRuntimeDir := C.CString(runtimeDir)
	defer C.free(unsafe.Pointer(cstrCountry))
	defer C.free(unsafe.Pointer(cstrConfigFile))
	defer C.free(unsafe.Pointer(cstrRuntimeDir))

	alpr := C.AlprInit(cstrCountry, cstrConfigFile, cstrRuntimeDir)
	return &Alpr{cAlpr: alpr}
}

func (alpr *Alpr) SetDetectRegion(detectRegion bool) {
	C.SetDetectRegion(alpr.cAlpr, bool2Cint(detectRegion))
}

func (alpr *Alpr) SetTopN(topN int) {
	C.SetTopN(alpr.cAlpr, C.int(topN))
}

func (alpr *Alpr) SetDefaultRegion(region string) {
	cstrRegion := C.CString(region)
	defer C.free(unsafe.Pointer(cstrRegion))
	C.SetDefaultRegion(alpr.cAlpr, cstrRegion)
}

func (alpr *Alpr) IsLoaded() bool {
	return cint2Bool(C.IsLoaded(alpr.cAlpr))
}

func GetVersion() string {
	return C.GoString(C.GetVersion())
}

func (alpr *Alpr) RecognizeByFilePath(filePath string) (AlprResults, error) {
	cstrFilePath := C.CString(filePath)
	defer C.free(unsafe.Pointer(cstrFilePath))
	stringResult := C.GoString(C.RecognizeByFilePath(alpr.cAlpr, cstrFilePath))
	fmt.Println(stringResult)

	var results AlprResults
	err := json.Unmarshal([]byte(stringResult), &results)

	return results, err
}

func (alpr *Alpr) RecognizeByBlob(imageBytes []byte) (AlprResults, error) {
	stringImageBytes := string(imageBytes)
	cstrImageBytes := C.CString(stringImageBytes)
	defer C.free(unsafe.Pointer(cstrImageBytes))
	stringResult := C.GoString(C.RecognizeByBlob(alpr.cAlpr, cstrImageBytes, C.int(len(imageBytes))))

	var results AlprResults
	err := json.Unmarshal([]byte(stringResult), &results)

	return results, err
}

func (alpr *Alpr) Unload() {
	C.Unload(alpr.cAlpr)
}
