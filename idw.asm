[BITS 64]
[DEFAULT ABS]

global idw

section .note.GNU-stack noalloc noexec nowrite progbits

section .text



;1) fill ymm0 register with up to 8 dimensions of the target point, put an according bitmask into ymm1
;2) calculate distance from each point to the target, store results on stack, use bitmask to get rid of unwanted data from ymm memory-to-register loads
;3) do the IDW formula itself{sum(value_i/distance_i)/sum(1/distance_i)}, SIMD optimized.
;4) do the IDW formula itself{sum(value_i/distance_i)/sum(1/distance_i)}, scalarly to handle the out-of-bounds case
;5) extract and calculate the end result



; idw asm implementation assumes that dimensions are <= 8


; rdi - struct answer_nearest ptr, rsi - point coords
idw:
	mov rax, qword [rdi] ; dims
	mov rdx, qword [rdi+8] ; Npoints
	lea rsi, [rsi+rax*4-4]

	mov r8, rax ; dims counter
	vpxor xmm0, xmm0 ; here well have the target points coordinates
	vpxor xmm1, xmm1 ; writemask for nearest points
	mov ecx, 0xFFFFFFFF
	._loop_fill_target_coords:
		pinsrd xmm0, dword [rsi], 0
		pinsrd xmm1, ecx, 0
		sub rsi, 4
		dec r8
		jz .skip_shift
			vpslldq ymm0, 4
			vpslldq ymm1, 4
		.skip_shift:
	jnz ._loop_fill_target_coords ; now target points coordinates are stored neatly in an AVX2 register
								  ; ymm1 has the writemask for the nearest points

	shl rdx, 2
	sub rsp, rdx ; reserve place on stack for distances
	shr rdx, 2

	mov r8, rdx ; points counter
	xor r9, r9  ; distances offset
	mov rcx, qword [rdi+16] ; get nearest points coords
	shl rax, 2 ; dims*=4 for shifting the nearest points pointer
	._loop_distance:
		vmovups ymm2, ymm0 ; copy A[i]
		vmovups ymm3, [rcx]
		vpand ymm3, ymm1 ; clear not needed entrys
		vsubps ymm3, ymm2 ; A[i] - B[i]
		vmulps ymm3, ymm3 ; (A[i] - B[i]) * (A[i] - B[i])
		vextractf128 xmm2, ymm3, 1 ; extract upper 128 bits
		vaddps xmm3, xmm2
		vhaddps xmm3, xmm3
		vhaddps xmm3, xmm3 ; get horizontal sum i.e. distance in xmm3
		vmovss dword [rsp+r9], xmm3
		add rcx, rax ; shift the nearest points coords pointer
		add r9, 4
		dec r8
	jnz ._loop_distance


	mov r8d, 0x3F800000 ; 1.0f
	vmovd xmm3, r8d
	vbroadcastss ymm3, xmm3 ; array of 1.0fs

	mov r8, rdx 
	shr r8, 3 ; r8 - number of SIMD iterations
	and rdx, 0x7 ; rdx - number of scalar iterations

	vpxor ymm4, ymm4 ; tmp (1/dist) SIMD
	vpxor ymm5, ymm5 ; result SIMD

	vpxor ymm2, ymm2 ; tmp (1/dist) scalar
	vpxor ymm6, ymm6 ; result scalar

	xor r9, r9
	mov rcx, qword [rdi+24] ; get the values array pointer

	test r8, r8
	jz .skip_SIMD

	._loop_idw_SIMD:
		vmovups ymm0, [rsp+r9] ; get the distances
		vmulps ymm0, ymm0 ; squared 
		vmovups ymm1, [rcx+r9] ; get the values
		vdivps ymm1, ymm0
		vdivps ymm0, ymm3, ymm0
		vaddps ymm4, ymm0
		vaddps ymm5, ymm1
		add r9, 32
		dec r8
	jnz ._loop_idw_SIMD

	.skip_SIMD:

	test rdx, rdx 
	jz .skip_scalar

	._loop_idw_scalar:
		vmovss xmm0, [rsp+r9]
		vmulss xmm0, xmm0 ; squared
		vmovss xmm1, [rcx+r9]
		vdivss xmm1, xmm0
		vdivss xmm0, xmm3, xmm0
		vaddss xmm2, xmm0
		vaddss xmm6, xmm1
		add r9, 4
		dec rdx
	jnz ._loop_idw_scalar

	.skip_scalar:

	vextractf128 xmm0, ymm4, 1 ; extract upper 128 bits
	vaddps xmm4, xmm0
	vhaddps xmm4, xmm4
	vhaddps xmm4, xmm4
	vextractf128 xmm0, ymm5, 1 ; extract upper 128 bits
	vaddps xmm5, xmm0
	vhaddps xmm5, xmm5
	vhaddps xmm5, xmm5

	vaddss xmm4, xmm2
	vaddss xmm5, xmm6

	vdivss xmm0, xmm5, xmm4
	mov rdx, qword [rdi+8]
	shl rdx, 2
	add rsp, rdx
ret
