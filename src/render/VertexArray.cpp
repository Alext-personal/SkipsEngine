#include "render/VertexArray.h"
VertexArray::VertexArray(VertexBuffer vbo, VertexLayout layout) {
	glGenVertexArrays(1, &m_ID);
	glBindVertexArray(m_ID); //TO DO https://docs.gl/gl4/glVertexAttribPointer
	//https://docs.gl/gl4/glEnableVertexAttribArray //ADDVBO Function maybe instead of constructor
}
VertexArray::~VertexArray() {
	glDeleteVertexArrays(1, &m_ID);
}
void VertexArray::Bind() const {

}
void VertexArrat::UnBind() const {

}