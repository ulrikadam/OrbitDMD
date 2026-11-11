AARCH = 64
RASPPI = 3
PREFIX64 = aarch64-none-elf-
DEFINE += -DARM_ALLOW_MULTI_CORE
# grands blocs reutilisables (sinon les blocs > 512 Ko liberes sont perdus)
DEFINE += -DHEAP_BLOCK_BUCKET_SIZES=0x40,0x400,0x1000,0x4000,0x10000,0x40000,0x80000,0x100000,0x400000,0x1000000
# USB interne seulement pour la prise reseau (haut debit) : pas besoin des 8000 interruptions/s
DEFINE += -DNO_USB_SOF_INTR
