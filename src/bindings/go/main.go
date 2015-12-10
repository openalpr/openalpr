package main

import (
	"fmt"
	"io/ioutil"
)

import "github.com/openalpr/openalpr/src/bindings/go/openalpr"

func main() {
	alpr := openalpr.NewAlpr("us", "", "../../../runtime_data")
	defer alpr.Unload()

	if !alpr.IsLoaded() {
		fmt.Println("OpenAlpr failed to load!")
		return
	}
	alpr.SetTopN(20)

	fmt.Println(alpr.IsLoaded())
	fmt.Println(openalpr.GetVersion())

	resultFromFilePath, err := alpr.RecognizeByFilePath("lp.jpg")
	if err != nil {
		fmt.Println(err)
	}
	fmt.Printf("%+v\n", resultFromFilePath)
	fmt.Printf("\n\n\n")

	imageBytes, err := ioutil.ReadFile("lp.jpg")
	if err != nil {
		fmt.Println(err)
	}
	resultFromBlob, err := alpr.RecognizeByBlob(imageBytes)
	fmt.Printf("%+v\n", resultFromBlob)
}
