FUBS := /home/daniel/Documents/fubs2
#FUBS_TARGET := Demos/Hanoi
#FUBS_TARGET := Shell
FUBS_TARGET := MinShell

fubs_genbook.bin: genbook.bin FORCE
	cd $(FUBS) && fubs run FUBS/MakeBoot/FuBoot @base_address:$$(printf '%x' $$(stat -c%s $(PWD)/genbook.bin)) @boot_out:output.img @symbol_out:boot.txt $(FUBS_TARGET)
	cat genbook.bin $(FUBS)/output.img > fubs_genbook.bin

fubs_genbook_el2.bin: genbook.bin FORCE
	cd $(FUBS) && fubs run FUBS/MakeBoot/FuBoot @base_address:a00000 @boot_out:output.img @symbol_out:boot.txt $(FUBS_TARGET)
	cat genbook.bin $(FUBS)/output.img > fubs_genbook_el2.bin

fubs_pinebook_el3.bin: pinebook.bin FORCE
	cd $(FUBS) && fubs run FUBS/MakeBoot/FuBoot @base_address:$$(printf '%x' $$(stat -c%s $(PWD)/pinebook.bin)) @boot_out:output.img @symbol_out:boot.txt $(FUBS_TARGET)
	cat pinebook.bin $(FUBS)/output.img > fubs_pinebook.bin

fubs_pinebook_el2.bin: pinebook.bin FORCE
	cd $(FUBS) && fubs run FUBS/MakeBoot/FuBoot @base_address:a00000 @boot_out:output.img @symbol_out:boot.txt $(FUBS_TARGET)
	cat pinebook.bin $(FUBS)/output.img > fubs_pinebook.bin

fubs_pinebook.img: makeboot.out pinebook-poc-ddr.bin fubs_pinebook_el3.bin
	./makeboot.out --v1 --ddr pinebook-poc-ddr.bin --os fubs_pinebook.bin -o fubs_pinebook.img

genbook_fubs.img: makeboot.out genbook-ddr.bin fubs_genbook_el2.bin
	./makeboot.out --v2 --ddr genbook-ddr.bin --os fubs_genbook_el2.bin -o genbook_fubs.img

uboot_genbook.bin: genbook.bin /home/daniel/Pulled/u-boot/u-boot.bin
	cat genbook.bin /home/daniel/Pulled/u-boot/u-boot.bin > uboot_genbook.bin

2usb3588: rock.out genbook-ddr.bin fubs_genbook.bin
	./rock.out --v2 --ddr genbook-ddr.bin --os fubs_genbook.bin

3usb3588: rock.out genbook-ddr.bin uboot_genbook.bin
	$(XROCK) maskrom genbook-ddr.bin uboot_genbook.bin --rc4-off

1usb3399: rock.out pinebook-ddr.bin fubs_pinebook_el2.bin
	./rock.out --v1 --ddr pinebook-ddr.bin --os fubs_pinebook.bin

FORCE:
