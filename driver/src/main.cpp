#include <ntifs.h>


#define DRIVER_NAME L"replicant"
#define SYMBOLIC_LINK L"\\DosDevices\\replicant"
#define DRIVER_PATH L"\\Device\\replicant"
#define DRIVER_FULL L"\\Driver\\replicant"

//------------------------------------------------------------------------------
// Forward declarations for functions from the Windows kernel ( Undocumented )
//------------------------------------------------------------------------------
extern "C" { //undocumented windows internal functions (exported by ntoskrnl)
    NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING DriverName, PDRIVER_INITIALIZE InitializationFunction);
    NTKERNELAPI NTSTATUS MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T ReturnSize);
}

//------------------------------------------------------------------------------
// Logging functionality encapsulated in its own namespace
//------------------------------------------------------------------------------
namespace logging {
    inline static void debug(const char* text) {;
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, text));
    }
} // namespace logging

//------------------------------------------------------------------------------
// Driver-specific definitions and entry point
//------------------------------------------------------------------------------
namespace driver {
    // message type that will be passed between user program and driver
	struct info_t {
        HANDLE target_pid = 0; //process id of process we want to read from / write to
        void* target_address = 0x0; //address in the target proces we want to read from / write to
        void* buffer_address = 0x0; //address in our usermode process to copy to (read mode) / read from (write mode)
        SIZE_T size = 0; //size of memory to copy between our usermode process and target process
        SIZE_T return_size = 0; //number of bytes successfully read / written
    };

    // IOCTL codes grouped in a nested namespace for clarity
    namespace codes {
        constexpr ULONG init = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
        constexpr ULONG read = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
        constexpr ULONG write = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    }

	// Function prototypes for the driver's IOCTL handlers
	NTSTATUS init_io(PDEVICE_OBJECT device_object, PIRP irp) {
		UNREFERENCED_PARAMETER(device_object);
        IoCompleteRequest(irp, IO_NO_INCREMENT);
		logging::debug("Received IOCTL_INIT\n");
        return irp->IoStatus.Status;
	}

	// Function prototypes for the driver's IOCTL handlers
    NTSTATUS close_io(PDEVICE_OBJECT device_obj, PIRP irp) {
        UNREFERENCED_PARAMETER(device_obj);

        IoCompleteRequest(irp, IO_NO_INCREMENT);
        return irp->IoStatus.Status;
    }

	// Function prototypes for the driver's IOCTL handlers
    NTSTATUS control_io(PDEVICE_OBJECT device_obj, PIRP irp) {
        UNREFERENCED_PARAMETER(device_obj);
		logging::debug("Received IOCTL\n");

        PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(irp);
        auto buffer = reinterpret_cast<info_t*>(irp->AssociatedIrp.SystemBuffer);

		if (stack == nullptr || buffer == nullptr) {
			IoCompleteRequest(irp, IO_NO_INCREMENT);
			return STATUS_UNSUCCESSFUL;
		}

		static PEPROCESS target_process = nullptr;

		const ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;

		// default status
		NTSTATUS status = STATUS_UNSUCCESSFUL;
		switch (code) {
		case codes::init:
			if (buffer->target_pid == 0) {
				logging::debug("Received IOCTL_INIT with invalid PID\n");
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			if (!NT_SUCCESS(PsLookupProcessByProcessId(buffer->target_pid, &target_process))) {
				logging::debug("Failed to find target process\n");
				status = STATUS_NOT_FOUND;
				break;
			}
         
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Target PID: %p\n", buffer->target_pid));
          
			break;
		case codes::read:
			if (target_process == nullptr) {
				logging::debug("Received IOCTL_READ without a target process\n");
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			if (buffer->target_address == nullptr || buffer->buffer_address == nullptr || buffer->size == 0) {
				logging::debug("Received IOCTL_READ with invalid parameters\n");
				status = STATUS_INVALID_PARAMETER;
				break;
			}
            {
                SIZE_T return_size = 0;
                status = MmCopyVirtualMemory(
                    target_process,
                    buffer->target_address,
                    PsGetCurrentProcess(),
                    buffer->buffer_address,
                    buffer->size,
                    KernelMode,
                    &return_size
                );
                if (!NT_SUCCESS(status)) {
                    logging::debug("Failed to read memory\n");
                }
                logging::debug("successfully to read memory\n");
                buffer->return_size = return_size;
            }
            break;
	
		case codes::write:
			if (target_process == nullptr) {
				logging::debug("Received IOCTL_WRITE without a target process\n");
                status = STATUS_INVALID_PARAMETER;
				break;
			}
			if (buffer->target_address == nullptr || buffer->buffer_address == nullptr || buffer->size == 0) {
				logging::debug("Received IOCTL_WRITE with invalid parameters\n");
                status = STATUS_INVALID_PARAMETER;
				break;
			}
            {
                SIZE_T return_size = 0;
                status = MmCopyVirtualMemory(
                    PsGetCurrentProcess(),
                    buffer->buffer_address,
                    target_process,
                    buffer->target_address,
                    buffer->size,
                    KernelMode,
                    &return_size
                );
                if (!NT_SUCCESS(status)) {
                    logging::debug("Failed to write memory\n");
                }
                logging::debug("successfully to write memory\n");
                buffer->return_size = return_size;
            }
            break;
		default:
            logging::debug("Received unknown IOCTL code\n");
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
		}
        irp->IoStatus.Status = status;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
        return irp->IoStatus.Status;
    }
    

} // namespace driver

// Main driver function
NTSTATUS real_main(PDRIVER_OBJECT driver_obj, PUNICODE_STRING registery_path) {
    logging::debug("Initializing driver\n");
    UNREFERENCED_PARAMETER(registery_path);
    UNICODE_STRING dev_name, sym_link;
    RtlInitUnicodeString(&dev_name, DRIVER_PATH);
	// Create device object
	PDEVICE_OBJECT dev_obj;
	NTSTATUS status = IoCreateDevice(driver_obj, 0, &dev_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &dev_obj);
    if (status != STATUS_SUCCESS) {
		logging::debug("Failed to create device object\n");
		return status;
    }

	logging::debug("Device object created\n");
	RtlInitUnicodeString(&sym_link, SYMBOLIC_LINK);
	status = IoCreateSymbolicLink(&sym_link, &dev_name);
    if (status != STATUS_SUCCESS) {
        logging::debug("Failed to create symbolic link\n");
        return status;
    }
	logging::debug("Symbolic link created\n");
    
    SetFlag(dev_obj->Flags, DO_BUFFERED_IO);

	// Set the driver's handler to our driver function
    driver_obj->MajorFunction[IRP_MJ_CREATE] = driver::init_io;
    driver_obj->MajorFunction[IRP_MJ_CLOSE] = driver::close_io;
    driver_obj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = driver::control_io;

	ClearFlag(dev_obj->Flags, DO_DEVICE_INITIALIZING);
	logging::debug("Driver success initialized\n");
    return status;
}

// Driver entry point
NTSTATUS DriverEntry() {
    UNICODE_STRING drv_name = {};
    RtlInitUnicodeString(&drv_name, DRIVER_FULL);
    logging::debug("Creating a driver\n");
    return IoCreateDriver(&drv_name, &real_main);
}