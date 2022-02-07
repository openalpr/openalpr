const labelMap = {
  1: { name: "ThumbsUp", color: "red" },
  2: { name: "ThumbsDown", color: "yellow" },
  3: { name: "ThankYou", color: "lime" },
  4: { name: "LiveLong", color: "blue" },
};

export const drawRect = (
  boxes,
  classes,
  scores,
  threshold,
  imgWidth,
  imgHeight,
  ctx
) => {
  for (let i = 0; i < boxes.length; i++) {
    if (classes[i] && scores[i] > threshold) {
      console.log(classes[i], scores[i]);
      const [y, x, height, width] = boxes[i];
      const text = classes[i];

      ctx.strokeStyle = labelMap[text]["color"];
      ctx.lineWidth = 10;
      ctx.fillStyle = "white";
      ctx.font = "30px Arial";

      ctx.beginPath();
      ctx.fillText(
        labelMap[text]["name"] + " - " + Math.round(scores[i] * 100) / 100,
        x * imgWidth,
        y * imgHeight - 10
      );
      ctx.rect(
        x * imgWidth,
        y * imgHeight,
        (width * imgWidth) / 2,
        (height * imgHeight) / 2
      );
      ctx.stroke();
    }
  }
};
