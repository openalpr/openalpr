import React, { useRef } from "react";
import Webcam from "react-webcam";
import "./App.css";
import { dataURLtoFile, uploadImage, beep } from "./util";

const URL = "https://192.168.1.5:5000/classify/";

function App() {
  const webcamRef = useRef(null);
  //const canvasRef = useRef(null);

  const tryPicture = () => {
    if (
      typeof webcamRef.current !== "undefined" &&
      webcamRef.current !== null &&
      webcamRef.current.video.readyState === 4
    ) {
      takePicture();
    } else {
      setTimeout(() => {
        tryPicture();
      }, 1000);
    }
  };

  const takePicture = () => {
    let file = dataURLtoFile(webcamRef.current.getScreenshot(), "file.jpg");

    uploadImage(file, URL)
      .then((res) => {
        res.json().then((json) => {
          if (json["cars"].length > 0) {
            beep.play();
          }
          console.log(json);
          takePicture();
        });
      })
      .catch((e) => {
        console.log("failed");
        setTimeout(() => {
          takePicture();
        }, 1000);
      });
  };

  /* useEffect(() => {
    takePicture();
  }, []); */

  return (
    <div className="App">
      <div className="container">
        <Webcam
          ref={webcamRef}
          onUserMedia={tryPicture()}
          muted={true}
          videoConstraints={{ facingMode: "environment" }}
          screenshotFormat="image/jpeg"
          style={{
            position: "absolute",
            marginLeft: "auto",
            marginRight: "auto",
            left: 0,
            right: 0,
            textAlign: "center",
            zindex: 9,
            width: 640,
            height: 480,
          }}
        />
        {/* <canvas
          ref={canvasRef}
          style={{
            position: "absolute",
            marginLeft: "auto",
            marginRight: "auto",
            left: 0,
            right: 0,
            textAlign: "center",
            zindex: 8,
            width: 640,
            height: 480,
          }}
        /> */}
      </div>
    </div>
  );
}

export default App;
