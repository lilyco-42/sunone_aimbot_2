# Troubleshooting Order

Use this order when something feels wrong:

1. Confirm build family: DML or CUDA.
2. Confirm model type: `.onnx` for DML, `.engine` for TensorRT, or `.onnx` when you want CUDA to generate a new engine.
3. Confirm capture is showing the expected content.
4. Confirm selected `input_method` has its required runtime/device.
5. Turn on useful logs or diagnostics.
6. Rebuild through the matching wrapper and run a real DML or CUDA smoke test after source changes.

More specific guides:

- [Backend selection and checks](backends.md)
- [UDP capture](udp-capture.md)
- [Capture diagnostics](capture-diagnostics.md)
- [Input methods](input-methods.md)
- [Overlay and GUI behavior](overlay.md)
- [Build workflow](build-workflow.md)
